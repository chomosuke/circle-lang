#include "runtime.hpp"

#include "diagnostic.hpp"
#include "parser.hpp"

#include <iostream>
#include <variant>

// #define DEBUG_OUTPUT

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
            m_elements.emplace(number::Index(number::Value(i), m_length),
                               from_ast(std::move(node.elements[i])));
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

    void Array::insert(std::vector<diag::WithInfo<number::Value>>&& indices,
                       std::unique_ptr<Obj> v) {
        auto* walk = this;
        auto last = std::move(indices.back());
        indices.pop_back();
        for (const auto& index : indices) {
            auto e = walk->m_elements.find(number::Index::make_ref(index.t, walk->m_length));
            if (e == walk->m_elements.end()) {
                throw_index_non_array(index.range);
            }
            walk = dynamic_cast<Array*>(e->second.get());
            if (walk == nullptr) {
                throw_index_non_array(index.range);
            }
        }
        walk->m_elements.insert_or_assign(number::Index(std::move(last.t), walk->m_length),
                                          std::move(v));
    }
    std::unique_ptr<Obj> Array::index(const number::Value& i) const {
        auto e = m_elements.find(number::Index::make_ref(i, m_length));
        if (e == m_elements.end()) {
            return std::make_unique<Number>(number::Value(1), std::nullopt);
        }
        return e->second->clone();
    }

    void Array::execute(Array& gca, std::istream& in, std::ostream& out) {
#ifdef DEBUG_OUTPUT
        std::cout << "Exe array length " << m_length << '\n';
#endif
        const auto zero = number::Value(BigInt(0));
        auto first = index(zero)->evaluate(gca);
        auto* number = dynamic_cast<Number*>(first.get());
        while (number == nullptr || !number::equal(number->get_value(), zero)) {
            for (auto i = 1; i < m_length; i++) {
                index(number::Value(i))->execute(gca, in, out);
            }

            first = index(zero)->evaluate(gca);
            number = dynamic_cast<Number*>(first.get());
        }
    }
    std::unique_ptr<Obj> Array::evaluate(const Array& /*gca*/) const { return clone(); }
    std::unique_ptr<Obj> Array::clone() const {
        auto na = std::make_unique<Array>(m_length, get_range());
        for (const auto& [i, e] : m_elements) {
            na->m_elements.emplace(i.clone(), e->clone());
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

    std::optional<std::vector<diag::WithInfo<number::Value>>>
    Index::get_gca_location(const Array& gca) const {
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
                loc->emplace_back(*ind_num->get_range(), ind_num->get_value().clone());
            }
            return loc;
        } else {
            auto loc = std::vector<diag::WithInfo<number::Value>>();
            loc.emplace_back(*ind_num->get_range(), ind_num->get_value().clone());
            return loc;
        }
    }

    void Index::execute(Array& gca, std::istream& in, std::ostream& out) {
        evaluate(gca)->execute(gca, in, out);
#ifdef DEBUG_OUTPUT
        std::cout << "Exe index" << '\n';
#endif
    }
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
#ifdef DEBUG_OUTPUT
        std::cout << "Eval index " << diag::to_string(ind_num->get_value()) << '\n';
#endif
        return arr->index(ind_num->get_value())->evaluate(gca);
    }
    std::unique_ptr<Obj> Index::clone() const { return clone_specialize(); }
    std::unique_ptr<Index> Index::clone_specialize() const {
        auto subject = std::optional<std::unique_ptr<Obj>>();
        if (m_subject) {
            subject = (*m_subject)->clone();
        }
        return std::unique_ptr<Index>(new Index(std::move(subject), m_index->clone(), get_range()));
    }

    Assign::Assign(std::unique_ptr<Index>&& lhs, std::unique_ptr<Obj>&& rhs,
                   std::optional<diag::Range> range)
        : Obj(range), m_lhs{std::move(lhs)}, m_rhs{std::move(rhs)} {}
    Assign::Assign(ast::Assign&& node, diag::Range range)
        : Obj(range), m_lhs{std::make_unique<Index>(std::move(*node.lhs.t), node.lhs.range)},
          m_rhs{from_ast(std::move(node.rhs))} {}

    void Assign::execute(Array& gca, std::istream& /*in*/, std::ostream& /*out*/) {
        auto rhs = m_rhs->evaluate(gca);
        auto gca_loc = m_lhs->get_gca_location(gca);
#ifdef DEBUG_OUTPUT
        std::cout << "Exe assign";
        if (gca_loc) {
            for (const auto& [_, v] : *gca_loc) {
                std::cout << ' ' << diag::to_string(v);
            }
        }
        std::cout << '\n';
#endif
        if (gca_loc) {
            gca.insert(std::move(*gca_loc), std::move(rhs));
        }
    }
    std::unique_ptr<Obj> Assign::evaluate(const Array& /*gca*/) const { return clone(); }
    std::unique_ptr<Obj> Assign::clone() const {
        return std::unique_ptr<Obj>(
            new Assign(m_lhs->clone_specialize(), m_rhs->clone(), get_range()));
    }

    OperatorBinary::OperatorBinary(number::op::Binary kind, std::unique_ptr<Obj>&& lhs,
                                   std::unique_ptr<Obj>&& rhs, std::optional<diag::Range> range)
        : Obj(range), m_kind{kind}, m_lhs{std::move(lhs)}, m_rhs{std::move(rhs)} {}

    OperatorBinary::OperatorBinary(ast::OperatorBinary&& node, diag::Range range)
        : Obj(range), m_kind{node.kind}, m_lhs{from_ast(std::move(node.lhs))},
          m_rhs{from_ast(std::move(node.rhs))} {}

    void OperatorBinary::execute(Array& gca, std::istream& in, std::ostream& out) {
        evaluate(gca)->execute(gca, in, out);
#ifdef DEBUG_OUTPUT
        std::cout << "Exe " << diag::to_string(m_kind) << '\n';
#endif
    }
    std::unique_ptr<Obj> OperatorBinary::evaluate(const Array& gca) const {
        auto rhs = m_rhs->evaluate(gca);
        auto* r = dynamic_cast<Number*>(rhs.get());
        auto lhs = m_lhs->evaluate(gca);
        auto* l = dynamic_cast<Number*>(lhs.get());
        if (l == nullptr || r == nullptr) {
            throw diag::RuntimeError{
                .msg{std::format("{} Can not operate on non number", get_range()->to_string())}};
        }

#ifdef DEBUG_OUTPUT
        std::cout << "Eval " << diag::to_string(l->get_value()) << ' ' << diag::to_string(m_kind)
                  << ' ' << diag::to_string(r->get_value()) << '\n';
#endif

        switch (m_kind) {
        case number::op::plus:
            return std::make_unique<Number>(l->get_value() + r->get_value(), std::nullopt);
        case number::op::minus:
            return std::make_unique<Number>(l->get_value() - r->get_value(), std::nullopt);
        case number::op::multiply:
            return std::make_unique<Number>(l->get_value() * r->get_value(), std::nullopt);
        case number::op::divide:
            return std::make_unique<Number>(l->get_value() / r->get_value(), std::nullopt);
        case number::op::bool_and:
            return std::make_unique<Number>(l->get_value() && r->get_value(), std::nullopt);
        case number::op::bool_or:
            return std::make_unique<Number>(l->get_value() || r->get_value(), std::nullopt);
        case number::op::equal:
            return std::make_unique<Number>(l->get_value() == r->get_value(), std::nullopt);
        case number::op::not_equal:
            return std::make_unique<Number>(l->get_value() != r->get_value(), std::nullopt);
        case number::op::smaller:
            return std::make_unique<Number>(l->get_value() < r->get_value(), std::nullopt);
        case number::op::smaller_or_equal:
            return std::make_unique<Number>(l->get_value() <= r->get_value(), std::nullopt);
        case number::op::greater:
            return std::make_unique<Number>(l->get_value() > r->get_value(), std::nullopt);
        case number::op::greater_or_equal:
            return std::make_unique<Number>(l->get_value() >= r->get_value(), std::nullopt);
        default:
            assert(false && "Operator not found");
        }
    }
    std::unique_ptr<Obj> OperatorBinary::clone() const {
        return std::unique_ptr<Obj>(
            new OperatorBinary(m_kind, m_lhs->clone(), m_rhs->clone(), get_range()));
    }

    OperatorUnary::OperatorUnary(number::op::Unary kind, std::unique_ptr<Obj>&& rhs,
                                 std::optional<diag::Range> range)
        : Obj(range), m_kind{kind}, m_rhs{std::move(rhs)} {}

    OperatorUnary::OperatorUnary(ast::OperatorUnary&& node, diag::Range range)
        : Obj(range), m_kind{node.kind}, m_rhs{from_ast(std::move(node.rhs))} {}

    void OperatorUnary::execute(Array& gca, std::istream& in, std::ostream& out) {
        evaluate(gca)->execute(gca, in, out);
#ifdef DEBUG_OUTPUT
        std::cout << "Exe " << diag::to_string(m_kind) << '\n';
#endif
    }
    std::unique_ptr<Obj> OperatorUnary::evaluate(const Array& gca) const {
        auto rhs = m_rhs->evaluate(gca);
        auto* r = dynamic_cast<Number*>(rhs.get());
        if (r == nullptr) {
            throw diag::RuntimeError{
                .msg{std::format("{} Can not operate on non number", get_range()->to_string())}};
        }

#ifdef DEBUG_OUTPUT
        std::cout << "Eval " << diag::to_string(m_kind) << '\n';
#endif

        switch (m_kind) {
        case number::op::bool_not:
            return std::make_unique<Number>(!r->get_value(), std::nullopt);
        }
    }
    std::unique_ptr<Obj> OperatorUnary::clone() const {
        return std::unique_ptr<Obj>(new OperatorUnary(m_kind, m_rhs->clone(), get_range()));
    }

    Number::Number(ast::Number&& node, diag::Range range)
        : Obj(range), m_value{std::move(node.value)} {}
    Number::Number(number::Value&& value, std::optional<diag::Range> range)
        : Obj(range), m_value{std::move(value)} {}

    const number::Value& Number::get_value() const { return m_value; }

    void Number::execute(Array& /*gca*/, std::istream& /*in*/, std::ostream& /*out*/) {
#ifdef DEBUG_OUTPUT
        std::cout << "Exe number" << '\n';
#endif
    }
    std::unique_ptr<Obj> Number::evaluate(const Array& /*gca*/) const { return clone(); }
    std::unique_ptr<Obj> Number::clone() const {
        return std::make_unique<Number>(m_value.clone(), get_range());
    }

    StdInput::StdInput() : Obj(std::nullopt) {}
    void StdInput::execute(Array& gca, std::istream& in, std::ostream& /*out*/) {
#ifdef DEBUG_OUTPUT
        std::cout << "Exe std_input" << '\n';
#endif
        auto chr = in.get();
        auto loc = std::vector<diag::WithInfo<number::Value>>();
        loc.emplace_back(diag::Range{}, number::Value("std_input_char"));
        gca.insert(std::move(loc), std::make_unique<Number>(number::Value(chr), std::nullopt));
    }
    [[nodiscard]] std::unique_ptr<Obj> StdInput::evaluate(const Array& /*gca*/) const {
        return clone();
    }
    [[nodiscard]] std::unique_ptr<Obj> StdInput::clone() const {
        return std::make_unique<StdInput>();
    }

    StdOutput::StdOutput() : Obj(std::nullopt) {}
    void StdOutput::execute(Array& gca, std::istream& /*in*/, std::ostream& out) {
#ifdef DEBUG_OUTPUT
        std::cout << "Exe std_output" << '\n';
#endif
        auto obj = gca.index(number::Value("std_output_char"));
        auto* num = dynamic_cast<Number*>(obj.get());
        if (num == nullptr) {
            throw diag::RuntimeError{.msg{"(std_output_char) isn't a number."}};
        }
        auto c = num->get_value().div_pi();
        if (!c) {
            throw diag::RuntimeError{.msg{"(std_output_char) isn't a multiple of pi."}};
        }
        if (*c < 0 || *c > CHAR_MAX) {
            throw diag::RuntimeError{.msg{"(std_output_char) isn't within the range of ascii value."}};
        }
        out << static_cast<char>(c->to_int());
    }
    [[nodiscard]] std::unique_ptr<Obj> StdOutput::evaluate(const Array& /*gca*/) const {
        return clone();
    }
    [[nodiscard]] std::unique_ptr<Obj> StdOutput::clone() const {
        return std::make_unique<StdOutput>();
    }

    Runtime::Runtime(ast::Array&& code)
        : m_gca{static_cast<int>(code.elements.size()), std::nullopt},
          m_code{std::move(code),
                 code.elements.empty()
                     ? diag::Range{.start{.line{0}, .column{0}}, .end{.line{0}, .column{0}}}
                     : diag::Range{.start{code.elements[0].range.start},
                                   .end{code.elements.back().range.end}}} {
        auto loc = std::vector<diag::WithInfo<number::Value>>();
        loc.emplace_back(diag::Range{}, number::Value("std_input"));
        m_gca.insert(std::move(loc), std::make_unique<StdInput>());
        loc = std::vector<diag::WithInfo<number::Value>>();
        loc.emplace_back(diag::Range{}, number::Value("std_output"));
        m_gca.insert(std::move(loc), std::make_unique<StdOutput>());
    }

    void Runtime::run(std::istream& in, std::ostream& out, std::ostream& err) {
        try {
            m_code.execute(m_gca, in, out);
        } catch (const diag::RuntimeError& e) {
            err << e.msg;
        }
    }
} // namespace runtime
