
#include "runtime.hpp"

#include "diagnostic.hpp"
#include "parser.hpp"

#include <algorithm>
#include <iostream>
#include <variant>

namespace runtime {
    std::unique_ptr<Obj> from_ast(ast::Node<ast::Any>&& node) {
        auto range = node.range;
        return std::visit(
            [&]<typename T>(T&& t) -> std::unique_ptr<Obj> {
                static_assert(std::is_same_v<T, std::decay_t<T>>);
                if constexpr (std::is_same_v<T, ast::Array>) {
                    return std::make_unique<Array>(std::forward<ast::Array>(t), range);
                } else if constexpr (std::is_same_v<T, ast::Assign>) {
                    return std::make_unique<Assign>(std::forward<ast::Assign>(t), range);
                } else if constexpr (std::is_same_v<T, ast::Index>) {
                    return std::make_unique<Index>(std::forward<ast::Index>(t), range);
                } else if constexpr (std::is_same_v<T, ast::OperatorBinary>) {
                    return std::make_unique<OperatorBinary>(std::forward<ast::OperatorBinary>(t),
                                                            range);
                } else if constexpr (std::is_same_v<T, ast::OperatorUnary>) {
                    return std::make_unique<OperatorUnary>(std::forward<ast::OperatorUnary>(t),
                                                           range);
                } else if constexpr (std::is_same_v<T, ast::Number>) {
                    return std::make_unique<Number>(std::forward<ast::Number>(t), range);
                } else {
                    static_assert(false, "Not exhaustive");
                }
            },
            std::move(*node.t));
    }

    Obj::Obj(std::optional<diag::Range> range) : m_range{range} {}

    std::optional<diag::Range> Obj::get_range() const { return m_range; }

    Array::Array(ast::Array&& node, diag::Range range)
        : Obj(range), m_length{static_cast<int>(node.elements.size())} {
        for (auto i = 0; i < node.elements.size(); i++) {
            m_elements.insert(std::make_pair(number::Index(number::Value(i), m_length),
                                             from_ast(std::move(node.elements[i]))));
        }
    }
    Array::Array(int length, std::optional<diag::Range> range) : Obj(range), m_length{length} {}

    void throw_index_non_array(diag::Range range) {
        throw diag::RuntimeError{
            .msg{std::format("{} Attempting to index non array object.", range.to_string())}};
    }
    void throw_index_non_number(diag::Range range) {
        throw diag::RuntimeError{.msg{
            std::format("{} Attempting to index an array with a non number.", range.to_string())}};
    }

    void Array::insert(std::vector<number::Value>&& indices, std::unique_ptr<Obj> v) {
        auto* walk = this;
        auto last = std::move(indices.back());
        indices.pop_back();
        for (const auto& index : indices) {
            auto e = walk->m_elements.find(number::Index(index, walk->m_length));
            if (e == walk->m_elements.end()) {
                throw_index_non_array(*walk->get_range());
            }
            walk = dynamic_cast<Array*>(e->second.get());
            if (walk == nullptr) {
                throw_index_non_array(*walk->get_range());
            }
        }
        walk->m_elements.insert(std::make_pair(number::Index(last, m_length), std::move(v)));
    }
    std::unique_ptr<Obj> Array::index(const number::Value& i) const {
        auto e = m_elements.find(number::Index(i, m_length));
        if (e == m_elements.end()) {
            return std::make_unique<Number>(number::Value(1), std::nullopt);
        }
        return e->second->clone();
    }

    void Array::execute(Array& gca) {
        const auto zero = number::Value(BigInt(0));
        auto first = index(zero)->evaluate(gca);
        auto* number = dynamic_cast<Number*>(first.get());
        while (number == nullptr || !number::equal(number->get_value(), zero)) {
            for (auto i = 0; i < m_length; i++) {
                index(number::Value(i))->execute(gca);
            }

            first = index(zero)->evaluate(gca);
            number = dynamic_cast<Number*>(first.get());
        }
    }
    std::unique_ptr<Obj> Array::evaluate(const Array& /*gca*/) const { return clone(); }
    std::unique_ptr<Obj> Array::clone() const {
        auto na = std::make_unique<Array>(m_length, get_range());
        for (const auto& [i, e] : m_elements) {
            na->m_elements.insert(std::make_pair(i.clone(), e->clone()));
        }
        return na;
    }

    Index::Index(std::optional<std::unique_ptr<Obj>>&& subject, std::unique_ptr<Obj>&& index,
                 std::optional<diag::Range> range)
        : Obj{range}, m_subject{std::move(subject)}, m_index{std::move(index)} {}

    Index::Index(ast::Index&& node, diag::Range range)
        : Obj(range),
          m_subject{node.subject ? std::make_optional(from_ast(
                                       ast::convert_node_variants<ast::Indexable, ast::Any>(
                                           std::move(*node.subject))))
                                 : std::nullopt},
          m_index{from_ast(std::move(node.index))} {}

    std::optional<std::vector<number::Value>> Index::get_gca_location(const Array& gca) const {
        auto index = m_index->evaluate(gca);
        auto* ind_num = dynamic_cast<Number*>(index.get());
        if (ind_num == nullptr) {
            throw_index_non_number(*get_range());
        }
        if (m_subject) {
            auto* subject = dynamic_cast<Index*>(m_subject->get());
            if (subject == nullptr) {
                if (dynamic_cast<Array*>(m_subject->get()) == nullptr) {
                    throw_index_non_array(*get_range());
                }
                return std::nullopt;
            }
            auto loc = subject->get_gca_location(gca);
            // push back ind_num
            if (loc) {
                loc->push_back(ind_num->get_value().clone());
            }
            return loc;
        } else {
            auto loc = std::vector<number::Value>();
            loc.push_back(ind_num->get_value().clone());
            return loc;
        }
    }

    void Index::execute(Array& /*gca*/) {}
    std::unique_ptr<Obj> Index::evaluate(const Array& gca) const {
        const auto* arr = &gca;
        // declaring this early so that it live as long as arr
        auto subject = std::unique_ptr<Obj>();
        if (m_subject) {
            subject = (*m_subject)->evaluate(gca);
            arr = dynamic_cast<Array*>(subject.get());
        }
        if (arr == nullptr) {
            throw_index_non_array(*get_range());
        }
        auto index = m_index->evaluate(gca);
        auto* ind_num = dynamic_cast<Number*>(index.get());
        if (ind_num == nullptr) {
            throw_index_non_number(*get_range());
        }
        return arr->index(ind_num->get_value());
    }
    std::unique_ptr<Obj> Index::clone() const {
        auto subject = std::optional<std::unique_ptr<Obj>>();
        if (m_subject) {
            subject = (*m_subject)->clone();
        }
        return std::unique_ptr<Index>(new Index(std::move(subject), m_index->clone(), get_range()));
    }

    Assign::Assign(ast::Assign&& node, diag::Range range)
        : Obj(range), m_lhs{std::make_unique<Index>(std::move(*node.lhs.t), node.lhs.range)},
          m_rhs{from_ast(std::move(node.rhs))} {}

    void Assign::execute(Array& gca) {
        auto rhs = m_rhs->evaluate(gca);
        auto gca_loc = m_lhs->get_gca_location(gca);
        if (gca_loc) {
            gca.insert(std::move(*gca_loc), std::move(rhs));
        }
    }
    std::unique_ptr<Obj> Assign::evaluate(const Array& gca) const {}
    std::unique_ptr<Obj> Assign::clone() const {}

    OperatorBinary::OperatorBinary(ast::OperatorBinary&& node, diag::Range range)
        : Obj(range), m_kind{node.kind}, m_lhs{from_ast(std::move(node.lhs))},
          m_rhs{from_ast(std::move(node.rhs))} {}

    void OperatorBinary::execute(Array& gca) {}
    std::unique_ptr<Obj> OperatorBinary::evaluate(const Array& gca) const {}
    std::unique_ptr<Obj> OperatorBinary::clone() const {}

    OperatorUnary::OperatorUnary(ast::OperatorUnary&& node, diag::Range range)
        : Obj(range), m_kind{node.kind}, m_rhs{from_ast(std::move(node.rhs))} {}

    void OperatorUnary::execute(Array& gca) {}
    std::unique_ptr<Obj> OperatorUnary::evaluate(const Array& gca) const {}
    std::unique_ptr<Obj> OperatorUnary::clone() const {}

    Number::Number(ast::Number&& node, diag::Range range)
        : Obj(range), m_value{std::move(node.value)} {}
    Number::Number(number::Value&& value, std::optional<diag::Range> range)
        : Obj(range), m_value{std::move(value)} {}

    void Number::execute(Array& gca) {}
    std::unique_ptr<Obj> Number::evaluate(const Array& gca) const {}
    std::unique_ptr<Obj> Number::clone() const {}

    Runtime::Runtime(ast::Array&& code)
        : m_gca{static_cast<int>(code.elements.size()), std::nullopt},
          m_code{std::move(code),
                 code.elements.empty()
                     ? diag::Range{.start{.line{0}, .column{0}}, .end{.line{0}, .column{0}}}
                     : diag::Range{.start{code.elements[0].range.start},
                                   .end{code.elements.back().range.end}}} {}

    void Runtime::run(std::istream& in, std::ostream& out, std::ostream& err) {}
} // namespace runtime
