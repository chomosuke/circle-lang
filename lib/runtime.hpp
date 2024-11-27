#pragma once

#include "diagnostic.hpp"
#include "macros.hpp"
#include "number.hpp"
#include "parser.hpp"

#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>

namespace runtime {
    template <bool DEBUG> class Obj;
    template <bool DEBUG> class Array;
    template <bool DEBUG> class Index;
    template <bool DEBUG> class Assign;
    template <bool DEBUG> class OperatorBinary;
    template <bool DEBUG> class OperatorUnary;
    template <bool DEBUG> class Number;

    template <bool DEBUG> std::unique_ptr<Obj<DEBUG>> from_ast(ast::Node<ast::Any>&& node) {
        auto range = node.range;
        return std::visit(
            [&]<typename T>(T&& t) -> std::unique_ptr<Obj<DEBUG>> {
                static_assert(std::is_same_v<T, std::decay_t<T>>);
                if constexpr (std::is_same_v<T, ast::Array>) {
                    return std::make_unique<Array<DEBUG>>(std::forward<ast::Array>(t), range);
                } else if constexpr (std::is_same_v<T, ast::Assign>) {
                    return std::make_unique<Assign<DEBUG>>(std::forward<ast::Assign>(t), range);
                } else if constexpr (std::is_same_v<T, ast::Index>) {
                    return std::make_unique<Index<DEBUG>>(std::forward<ast::Index>(t), range);
                } else if constexpr (std::is_same_v<T, ast::OperatorBinary>) {
                    return std::make_unique<OperatorBinary<DEBUG>>(
                        std::forward<ast::OperatorBinary>(t), range);
                } else if constexpr (std::is_same_v<T, ast::OperatorUnary>) {
                    return std::make_unique<OperatorUnary<DEBUG>>(
                        std::forward<ast::OperatorUnary>(t), range);
                } else if constexpr (std::is_same_v<T, ast::Number>) {
                    return std::make_unique<Number<DEBUG>>(std::forward<ast::Number>(t), range);
                } else {
                    static_assert(false, "Not exhaustive");
                }
            },
            std::move(*node.t));
    }

    inline std::vector<std::string> split(std::string s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        tokens.push_back(s);

        return tokens;
    }

    template <bool DEBUG> class Debugger {
      private:
        int m_arr_level{0};
        std::optional<int> m_stepping_level{1};
        std::unordered_set<int> m_breakpoints;
        std::vector<std::string> m_lines;

      public:
        explicit Debugger(const std::string& src_code) {
            if constexpr (DEBUG) {
                m_lines = split(src_code, "\n");
            }
        }
        void arr_enter() {
            if constexpr (DEBUG) {
                m_arr_level++;
            }
        }
        void arr_exit() {
            if constexpr (DEBUG) {
                m_arr_level--;
            }
        }
        void execute(const std::unique_ptr<Obj<DEBUG>>& obj, const Array<DEBUG>& gca);
    };

    template <bool DEBUG> class Obj {
      private:
        std::optional<diag::Range> m_range;

      public:
        Obj(const Obj&) = delete;
        Obj(Obj&&) = default;
        Obj& operator=(const Obj&) = delete;
        Obj& operator=(Obj&&) = default;
        virtual ~Obj() = default;

        explicit Obj(std::optional<diag::Range> range) : m_range{range} {}

        [[nodiscard]] std::optional<diag::Range> get_range() const { return m_range; }

        virtual void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& out,
                             Debugger<DEBUG>& debugger) = 0;
        [[nodiscard]] virtual std::unique_ptr<Obj<DEBUG>>
        evaluate(const Array<DEBUG>& gca) const = 0;
        [[nodiscard]] virtual std::unique_ptr<Obj<DEBUG>> clone() const = 0;
        [[nodiscard]] virtual std::string debug_string(int indent) const = 0;
    };

    inline void throw_index_non_array(diag::Range range) {
        throw diag::RuntimeError{
            .msg{std::format("{} Attempting to index non array object.", range.to_string())}};
    }
    inline void throw_index_non_number(diag::Range range) {
        throw diag::RuntimeError{.msg{
            std::format("{} Attempting to index an array with a non number.", range.to_string())}};
    }
    inline void print_indent(std::stringstream& ss, int indent) {
        for (auto i = 0; i < indent; i++) {
            ss << "    ";
        }
    }

    template <bool DEBUG> class Array : public Obj<DEBUG> {
      private:
        int m_length;
        std::unordered_map<number::Index, std::unique_ptr<Obj<DEBUG>>> m_elements;

      public:
        Array(ast::Array&& node, diag::Range range)
            : Obj<DEBUG>(range), m_length{static_cast<int>(node.elements.size())} {
            for (auto i = 0; i < node.elements.size(); i++) {
                m_elements.emplace(number::Index(number::Value(i), m_length),
                                   from_ast<DEBUG>(std::move(node.elements[i])));
            }
        }
        Array(int length, std::optional<diag::Range> range) : Obj<DEBUG>(range), m_length{length} {}

        void insert(std::vector<diag::WithInfo<number::Value>>&& indices,
                    std::unique_ptr<Obj<DEBUG>> v) {
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
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> index(const number::Value& i) const {
            auto e = m_elements.find(number::Index::make_ref(i, m_length));
            if (e == m_elements.end()) {
                return std::make_unique<Number<DEBUG>>(number::Value(1), std::nullopt);
            }
            return e->second->clone();
        }

        void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& out,
                     Debugger<DEBUG>& debugger) override {
            debugger.arr_enter();
            const auto zero = number::Value(BigInt(0));
            auto obj = index(zero);
            auto first = obj->evaluate(gca);
            debugger.execute(obj, gca);
            auto* number = dynamic_cast<Number<DEBUG>*>(first.get());
            while (number == nullptr || !number::equal(number->get_value(), zero)) {
                for (auto i = 1; i < m_length; i++) {
                    auto obj = index(number::Value(i));
                    debugger.execute(obj, gca);
                    obj->execute(gca, in, out, debugger);
                }

                auto obj = index(zero);
                first = obj->evaluate(gca);
                debugger.execute(obj, gca);
                number = dynamic_cast<Number<DEBUG>*>(first.get());
            }
            debugger.arr_exit();
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> evaluate(const Array& /*gca*/) const override {
            return clone();
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            auto na = std::make_unique<Array<DEBUG>>(m_length, this->get_range());
            for (const auto& [i, e] : m_elements) {
                na->m_elements.emplace(i.clone(), e->clone());
            }
            return na;
        }
        [[nodiscard]] std::string debug_string(int indent) const override {
            auto ss = std::stringstream();
            ss << "((\n";
            for (const auto& [i, e] : m_elements) {
                print_indent(ss, indent + 1);
                ss << diag::to_string(i.get_value()) << ": " << e->debug_string(indent + 1)
                   << ";\n";
            }
            print_indent(ss, indent);
            ss << "))";
            return ss.str();
        }
    };

    template <bool DEBUG> class Index : public Obj<DEBUG> {
      private:
        std::optional<std::unique_ptr<Obj<DEBUG>>> m_subject;
        std::unique_ptr<Obj<DEBUG>> m_index;
        Index(std::optional<std::unique_ptr<Obj<DEBUG>>>&& subject,
              std::unique_ptr<Obj<DEBUG>>&& index, std::optional<diag::Range> range)
            : Obj<DEBUG>{range}, m_subject{std::move(subject)}, m_index{std::move(index)} {}

      public:
        Index(ast::Index&& node, diag::Range range)
            : Obj<DEBUG>(range),
              m_subject{node.subject ? std::make_optional(from_ast<DEBUG>(
                                           ast::convert_node_variants<ast::Indexable, ast::Any>(
                                               std::move(*node.subject))))
                                     : std::nullopt},
              m_index{from_ast<DEBUG>(std::move(node.index))} {}

        [[nodiscard]] std::optional<std::vector<diag::WithInfo<number::Value>>>
        get_gca_location(const Array<DEBUG>& gca) const {
            auto index = m_index->evaluate(gca);
            auto* ind_num = dynamic_cast<Number<DEBUG>*>(index.get());
            if (ind_num == nullptr) {
                throw_index_non_number(*this->get_range());
            }
            if (m_subject) {
                auto* subject = dynamic_cast<Index*>(m_subject->get());
                if (subject == nullptr) {
                    if (dynamic_cast<Array<DEBUG>*>(m_subject->get()) == nullptr) {
                        throw_index_non_array(*this->get_range());
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

        [[nodiscard]] std::unique_ptr<Index> clone_specialize() const {
            auto subject = std::optional<std::unique_ptr<Obj<DEBUG>>>();
            if (m_subject) {
                subject = (*m_subject)->clone();
            }
            return std::unique_ptr<Index>(
                new Index(std::move(subject), m_index->clone(), this->get_range()));
        }

        void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& out,
                     Debugger<DEBUG>& debugger) override {
            evaluate(gca)->execute(gca, in, out, debugger);
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> evaluate(const Array<DEBUG>& gca) const override {
            const auto* arr = &gca;
            // declaring this early so that it live as long as arr
            auto subject = std::unique_ptr<Obj<DEBUG>>();
            if (m_subject) {
                subject = (*m_subject)->evaluate(gca);
                arr = dynamic_cast<Array<DEBUG>*>(subject.get());
            }
            if (arr == nullptr) {
                throw_index_non_array(*this->get_range());
            }
            auto index = m_index->evaluate(gca);
            auto* ind_num = dynamic_cast<Number<DEBUG>*>(index.get());
            if (ind_num == nullptr) {
                throw_index_non_number(*this->get_range());
            }
            return arr->index(ind_num->get_value())->evaluate(gca);
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return clone_specialize();
        }
        [[nodiscard]] std::string debug_string(int indent) const override {
            if (m_subject) {
                return std::format("{}( {} )", (*m_subject)->debug_string(indent),
                                   m_index->debug_string(indent));
            } else {
                return std::format("( {} )", m_index->debug_string(indent));
            }
        }
    };

    template <bool DEBUG> class Assign : public Obj<DEBUG> {
      private:
        std::unique_ptr<Index<DEBUG>> m_lhs;
        std::unique_ptr<Obj<DEBUG>> m_rhs;
        Assign(std::unique_ptr<Index<DEBUG>>&& lhs, std::unique_ptr<Obj<DEBUG>>&& rhs,
               std::optional<diag::Range> range)
            : Obj<DEBUG>(range), m_lhs{std::move(lhs)}, m_rhs{std::move(rhs)} {}

      public:
        Assign(ast::Assign&& node, diag::Range range)
            : Obj<DEBUG>(range),
              m_lhs{std::make_unique<Index<DEBUG>>(std::move(*node.lhs.t), node.lhs.range)},
              m_rhs{from_ast<DEBUG>(std::move(node.rhs))} {}

        void execute(Array<DEBUG>& gca, std::istream& /*in*/, std::ostream& /*out*/,
                     Debugger<DEBUG>& /*debugger*/) override {
            auto rhs = m_rhs->evaluate(gca);
            auto gca_loc = m_lhs->get_gca_location(gca);
            if (gca_loc) {
                gca.insert(std::move(*gca_loc), std::move(rhs));
            }
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>>
        evaluate(const Array<DEBUG>& /*gca*/) const override {
            return clone();
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return std::unique_ptr<Obj<DEBUG>>(
                new Assign(m_lhs->clone_specialize(), m_rhs->clone(), this->get_range()));
        }
        [[nodiscard]] std::string debug_string(int indent) const override {
            return std::format("{} := {}", m_lhs->debug_string(indent),
                               m_rhs->debug_string(indent));
        }
    };

    template <bool DEBUG> class OperatorBinary : public Obj<DEBUG> {
      private:
        number::op::Binary m_kind;
        std::unique_ptr<Obj<DEBUG>> m_lhs;
        std::unique_ptr<Obj<DEBUG>> m_rhs;
        OperatorBinary(number::op::Binary kind, std::unique_ptr<Obj<DEBUG>>&& lhs,
                       std::unique_ptr<Obj<DEBUG>>&& rhs, std::optional<diag::Range> range)
            : Obj<DEBUG>(range), m_kind{kind}, m_lhs{std::move(lhs)}, m_rhs{std::move(rhs)} {}

      public:
        OperatorBinary(ast::OperatorBinary&& node, diag::Range range)
            : Obj<DEBUG>(range), m_kind{node.kind}, m_lhs{from_ast<DEBUG>(std::move(node.lhs))},
              m_rhs{from_ast<DEBUG>(std::move(node.rhs))} {}

        void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& out,
                     Debugger<DEBUG>& debugger) override {
            evaluate(gca)->execute(gca, in, out, debugger);
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> evaluate(const Array<DEBUG>& gca) const override {
            auto rhs = m_rhs->evaluate(gca);
            auto* r = dynamic_cast<Number<DEBUG>*>(rhs.get());
            auto lhs = m_lhs->evaluate(gca);
            auto* l = dynamic_cast<Number<DEBUG>*>(lhs.get());
            if (l == nullptr || r == nullptr) {
                throw diag::RuntimeError{.msg{std::format("{} Can not operate on non number",
                                                          this->get_range()->to_string())}};
            }

            switch (m_kind) {
            case number::op::plus:
                return std::make_unique<Number<DEBUG>>(l->get_value() + r->get_value(),
                                                       std::nullopt);
            case number::op::minus:
                return std::make_unique<Number<DEBUG>>(l->get_value() - r->get_value(),
                                                       std::nullopt);
            case number::op::multiply:
                return std::make_unique<Number<DEBUG>>(l->get_value() * r->get_value(),
                                                       std::nullopt);
            case number::op::divide:
                return std::make_unique<Number<DEBUG>>(l->get_value() / r->get_value(),
                                                       std::nullopt);
            case number::op::bool_and:
                return std::make_unique<Number<DEBUG>>(l->get_value() && r->get_value(),
                                                       std::nullopt);
            case number::op::bool_or:
                return std::make_unique<Number<DEBUG>>(l->get_value() || r->get_value(),
                                                       std::nullopt);
            case number::op::equal:
                return std::make_unique<Number<DEBUG>>(l->get_value() == r->get_value(),
                                                       std::nullopt);
            case number::op::not_equal:
                return std::make_unique<Number<DEBUG>>(l->get_value() != r->get_value(),
                                                       std::nullopt);
            case number::op::smaller:
                return std::make_unique<Number<DEBUG>>(l->get_value() < r->get_value(),
                                                       std::nullopt);
            case number::op::smaller_or_equal:
                return std::make_unique<Number<DEBUG>>(l->get_value() <= r->get_value(),
                                                       std::nullopt);
            case number::op::greater:
                return std::make_unique<Number<DEBUG>>(l->get_value() > r->get_value(),
                                                       std::nullopt);
            case number::op::greater_or_equal:
                return std::make_unique<Number<DEBUG>>(l->get_value() >= r->get_value(),
                                                       std::nullopt);
            default:
                assert(false && "Operator not found");
            }
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return std::unique_ptr<Obj<DEBUG>>(
                new OperatorBinary(m_kind, m_lhs->clone(), m_rhs->clone(), this->get_range()));
        }
        [[nodiscard]] std::string debug_string(int indent) const override {
            return std::format("{} {} {}", m_lhs->debug_string(indent), diag::to_string(m_kind),
                               m_rhs->debug_string(indent));
        }
    };

    template <bool DEBUG> class OperatorUnary : public Obj<DEBUG> {
      private:
        number::op::Unary m_kind;
        std::unique_ptr<Obj<DEBUG>> m_rhs;
        OperatorUnary(number::op::Unary kind, std::unique_ptr<Obj<DEBUG>>&& rhs,
                      std::optional<diag::Range> range)
            : Obj<DEBUG>(range), m_kind{kind}, m_rhs{std::move(rhs)} {}

      public:
        OperatorUnary(ast::OperatorUnary&& node, diag::Range range)
            : Obj<DEBUG>(range), m_kind{node.kind}, m_rhs{from_ast<DEBUG>(std::move(node.rhs))} {}

        void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& out,
                     Debugger<DEBUG>& debugger) override {
            evaluate(gca)->execute(gca, in, out, debugger);
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> evaluate(const Array<DEBUG>& gca) const override {
            auto rhs = m_rhs->evaluate(gca);
            auto* r = dynamic_cast<Number<DEBUG>*>(rhs.get());
            if (r == nullptr) {
                throw diag::RuntimeError{.msg{std::format("{} Can not operate on non number",
                                                          this->get_range()->to_string())}};
            }

            switch (m_kind) {
            case number::op::bool_not:
                return std::make_unique<Number<DEBUG>>(!r->get_value(), std::nullopt);
            }
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return std::unique_ptr<Obj<DEBUG>>(
                new OperatorUnary(m_kind, m_rhs->clone(), this->get_range()));
        }
        [[nodiscard]] std::string debug_string(int indent) const override {
            return std::format("{}{}", diag::to_string(m_kind), m_rhs->debug_string(indent));
        }
    };

    template <bool DEBUG> class Number : public Obj<DEBUG> {
      private:
        number::Value m_value;

      public:
        Number(ast::Number&& node, diag::Range range)
            : Obj<DEBUG>(range), m_value{std::move(node.value)} {}
        Number(number::Value&& value, std::optional<diag::Range> range)
            : Obj<DEBUG>(range), m_value{std::move(value)} {}

        [[nodiscard]] const number::Value& get_value() const { return m_value; }

        void execute(Array<DEBUG>& /*gca*/, std::istream& /*in*/, std::ostream& /*out*/,
                     Debugger<DEBUG>& /*debugger*/) override {}
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>>
        evaluate(const Array<DEBUG>& /*gca*/) const override {
            return clone();
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return std::make_unique<Number<DEBUG>>(m_value.clone(), this->get_range());
        }
        [[nodiscard]] std::string debug_string(int /*indent*/) const override {
            return diag::to_string(m_value);
        }
    };

    template <typename T, bool DEBUG> class StdFun : public Obj<DEBUG> {
      public:
        // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
        StdFun() : Obj<DEBUG>(std::nullopt) {}
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>>
        evaluate(const Array<DEBUG>& /*gca*/) const override {
            return clone();
        }
        [[nodiscard]] std::unique_ptr<Obj<DEBUG>> clone() const override {
            return std::make_unique<T>();
        }
    };

    template <bool DEBUG> class StdInput : public StdFun<StdInput<DEBUG>, DEBUG> {
      public:
        void execute(Array<DEBUG>& gca, std::istream& in, std::ostream& /*out*/,
                     Debugger<DEBUG>& /*debugger*/) override {
            auto chr = in.get();
            auto loc = std::vector<diag::WithInfo<number::Value>>();
            loc.emplace_back(diag::Range{}, number::Value("std_input_char"));
            gca.insert(std::move(loc),
                       std::make_unique<Number<DEBUG>>(number::Value(chr), std::nullopt));
        }
        [[nodiscard]] std::string debug_string(int /*indent*/) const override {
            return "std_input";
        }
    };

    template <bool DEBUG> class StdOutput : public StdFun<StdOutput<DEBUG>, DEBUG> {
      public:
        void execute(Array<DEBUG>& gca, std::istream& /*in*/, std::ostream& out,
                     Debugger<DEBUG>& /*debug*/) override {
            auto obj = gca.index(number::Value("std_output_char"));
            auto* num = dynamic_cast<Number<DEBUG>*>(obj.get());
            if (num == nullptr) {
                throw diag::RuntimeError{.msg{"(std_output_char) isn't a number."}};
            }
            auto c = num->get_value().div_pi();
            if (!c) {
                throw diag::RuntimeError{.msg{"(std_output_char) isn't a multiple of pi."}};
            }
            if (*c < 0 || *c > CHAR_MAX) {
                throw diag::RuntimeError{
                    .msg{"(std_output_char) isn't within the range of ascii value."}};
            }
            out << static_cast<char>(c->to_int());
        }
        [[nodiscard]] std::string debug_string(int /*indent*/) const override {
            return "std_output";
        }
    };

    template <bool DEBUG> class StdDecompose : public StdFun<StdDecompose<DEBUG>, DEBUG> {
      public:
        void execute(Array<DEBUG>& gca, std::istream& /*in*/, std::ostream& /*out*/,
                     Debugger<DEBUG>& /*debugger*/) override {
            auto num = gca.index(number::Value("std_decompose_number"));
            const auto& number = dynamic_cast<Number<DEBUG>*>(num.get())->get_value();

            auto insert = [&](const std::vector<BigInt>& ator, std::string_view index) {
                auto arr = Array<DEBUG>(static_cast<int>(std::max(ator.size(), 1UL)), std::nullopt);
                if (ator.empty()) {
                    auto loc = std::vector<diag::WithInfo<number::Value>>();
                    loc.emplace_back(diag::Range{}, number::Value(BigInt(0)));
                    arr.insert(std::move(loc), std::make_unique<Number<DEBUG>>(
                                                   number::Value(BigInt(0)), std::nullopt));
                } else {
                    for (auto i = 0; i < ator.size(); i++) {
                        auto loc = std::vector<diag::WithInfo<number::Value>>();
                        loc.emplace_back(diag::Range{}, number::Value(BigInt(i)));
                        arr.insert(std::move(loc), std::make_unique<Number<DEBUG>>(
                                                       number::Value(ator[i]), std::nullopt));
                    }
                }

                auto loc = std::vector<diag::WithInfo<number::Value>>();
                loc.emplace_back(diag::Range{}, number::Value(index));
                gca.insert(std::move(loc), std::make_unique<Array<DEBUG>>(std::move(arr)));
            };

            insert(number.get_numerator(), "std_decompose_numerator");
            insert(number.get_denominator(), "std_decompose_denominator");
        }
        [[nodiscard]] std::string debug_string(int /*indent*/) const override {
            return "std_decompose";
        }
    };

    template <bool DEBUG> class Runtime {
      private:
        // glocal circular array
        Array<DEBUG> m_gca;
        Array<DEBUG> m_code;
        const std::string& m_src_code;

      public:
        explicit Runtime(ast::Array&& code, const std::string& src_code)
            : m_gca{static_cast<int>(code.elements.size()), std::nullopt},
              m_code{std::move(code),
                     code.elements.empty()
                         ? diag::Range{.start{.line{0}, .column{0}}, .end{.line{0}, .column{0}}}
                         : diag::Range{.start{code.elements[0].range.start},
                                       .end{code.elements.back().range.end}}},
              m_src_code{src_code} {
            auto loc = std::vector<diag::WithInfo<number::Value>>();
            loc.emplace_back(diag::Range{}, number::Value("std_input"));
            m_gca.insert(std::move(loc), std::make_unique<StdInput<DEBUG>>());
            loc = std::vector<diag::WithInfo<number::Value>>();
            loc.emplace_back(diag::Range{}, number::Value("std_output"));
            m_gca.insert(std::move(loc), std::make_unique<StdOutput<DEBUG>>());
            loc = std::vector<diag::WithInfo<number::Value>>();
            loc.emplace_back(diag::Range{}, number::Value("std_decompose"));
            m_gca.insert(std::move(loc), std::make_unique<StdDecompose<DEBUG>>());
        }

        void run(std::istream& in, std::ostream& out, std::ostream& err) {
            try {
                auto debugger = Debugger<DEBUG>(m_src_code);
                m_code.execute(m_gca, in, out, debugger);
            } catch (const diag::RuntimeError& e) {
                err << e.msg << '\n';
            }
        }
    };

    /*
     * The debugger support step over, step into, and step out, where the things stepping in and out
     * are Array executions.
     * Pause every array element and prints out the current code being executed.
     * i: step into
     * o: step out
     * n: step over
     * e Expr: evaluate an expression
     * b int: set breakpoint
     * c: continue;
     */
    template <bool DEBUG>
    void Debugger<DEBUG>::execute(const std::unique_ptr<Obj<DEBUG>>& obj, const Array<DEBUG>& gca) {
        if constexpr (DEBUG) {
            auto range = obj->get_range();
            if (m_stepping_level.value_or(0) >= m_arr_level ||
                (range && m_breakpoints.contains(range->start.line))) {
                std::cout << m_lines[range->start.line] << '\n';
                char act{0};
                auto con = true;
                while (con) {
                    act = static_cast<char>(std::cin.get());
                    while (act == '\n') {
                        act = static_cast<char>(std::cin.get());
                    }
                    switch (act) {
                    case 'i':
                        m_stepping_level = m_arr_level + 1;
                        con = false;
                        break;
                    case 'o':
                        m_stepping_level = m_arr_level - 1;
                        con = false;
                        break;
                    case 'n':
                        con = false;
                        m_stepping_level = m_arr_level;
                        break;
                    case 'c':
                        con = false;
                        m_stepping_level = std::nullopt;
                        break;
                    case 'b': {
                        auto line = 0;
                        std::cin >> line;
                        m_breakpoints.insert(line - 1);
                    } break;
                    case 'e': {
                        std::string expr;
                        std::getline(std::cin, expr);
                        act = '\n';

                        auto diags = diag::Diags();
                        auto parsed = parse(expr, diags);
                        if (!diags.empty()) {
                            std::cout << diags.to_string() << '\n';
                            if (diags.has_fatal()) {
                                return;
                            }
                        }
                        for (auto&& e : std::move(parsed->elements)) {
                            try {
                                std::cout
                                    << from_ast<DEBUG>(std::move(e))->evaluate(gca)->debug_string(0)
                                    << ";\n";
                            } catch (const diag::RuntimeError& e) {
                                std::cerr << e.msg << '\n';
                            }
                        }
                    } break;
                    case 'g':
                        std::cout << gca.debug_string(0) << '\n';
                        break;
                    default: {
                        std::string expr;
                        std::getline(std::cin, expr);
                        act = '\n';
                        std::cerr << "Unrecognized command.\n";
                    } break;
                    }
                    while (act != '\n') {
                        act = static_cast<char>(std::cin.get());
                    }
                }
            }
        }
    }
} // namespace runtime
