#pragma once

#include "lib/diagnostic.hpp"
#include "macros.hpp"
#include "number.hpp"
#include "parser.hpp"

#include <memory>

namespace runtime {
    class Array;
    class Index;
    class Assign;
    class OperatorBinary;
    class OperatorUnary;
    class Number;

    class Obj {
      private:
        std::optional<diag::Range> m_range;

      public:
        Obj(const Obj&) = delete;
        Obj(Obj&&) = default;
        Obj& operator=(const Obj&) = delete;
        Obj& operator=(Obj&&) = default;
        virtual ~Obj() = default;

        explicit Obj(std::optional<diag::Range> range);

        [[nodiscard]] std::optional<diag::Range> get_range() const;

        virtual void execute(Array& gca, std::istream& in, std::ostream& out) = 0;
        [[nodiscard]] virtual std::unique_ptr<Obj> evaluate(const Array& gca) const = 0;
        [[nodiscard]] virtual std::unique_ptr<Obj> clone() const = 0;
    };

    class Array : public Obj {
      private:
        int m_length;
        std::unordered_map<number::Index, std::unique_ptr<Obj>> m_elements;

      public:
        Array(int length, std::optional<diag::Range> range);
        Array(ast::Array&& node, diag::Range range);

        void insert(std::vector<diag::WithInfo<number::Value>>&& indices, std::unique_ptr<Obj> v);
        std::unique_ptr<Obj> index(const number::Value& i) const;

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        std::unique_ptr<Obj> clone() const override;
    };

    class Index : public Obj {
      private:
        std::optional<std::unique_ptr<Obj>> m_subject;
        std::unique_ptr<Obj> m_index;
        Index(std::optional<std::unique_ptr<Obj>>&& subject, std::unique_ptr<Obj>&& index,
              std::optional<diag::Range> range);

      public:
        Index(ast::Index&& node, diag::Range range);

        [[nodiscard]] std::optional<std::vector<diag::WithInfo<number::Value>>>
        get_gca_location(const Array& gca) const;

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
        [[nodiscard]] std::unique_ptr<Index> clone_specialize() const;
    };

    class Assign : public Obj {
      private:
        std::unique_ptr<Index> m_lhs;
        std::unique_ptr<Obj> m_rhs;
        Assign(std::unique_ptr<Index>&& lhs, std::unique_ptr<Obj>&& rhs,
               std::optional<diag::Range> range);

      public:
        Assign(ast::Assign&& node, diag::Range range);

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class OperatorBinary : public Obj {
      private:
        number::op::Binary m_kind;
        std::unique_ptr<Obj> m_lhs;
        std::unique_ptr<Obj> m_rhs;
        OperatorBinary(number::op::Binary kind, std::unique_ptr<Obj>&& lhs,
                       std::unique_ptr<Obj>&& rhs, std::optional<diag::Range> range);

      public:
        OperatorBinary(ast::OperatorBinary&& node, diag::Range range);

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class OperatorUnary : public Obj {
      private:
        number::op::Unary m_kind;
        std::unique_ptr<Obj> m_rhs;
        OperatorUnary(number::op::Unary kind, std::unique_ptr<Obj>&& rhs,
                      std::optional<diag::Range> range);

      public:
        OperatorUnary(ast::OperatorUnary&& node, diag::Range range);

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class Number : public Obj {
      private:
        number::Value m_value;

      public:
        Number(ast::Number&& node, diag::Range range);
        Number(number::Value&& value, std::optional<diag::Range> range);

        [[nodiscard]] const number::Value& get_value() const;

        void execute(Array& gca, std::istream& in, std::ostream& out) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    template <typename T> class StdFun : public Obj {
      public:
        // NOLINTNEXTLINE(bugprone-crtp-constructor-accessibility)
        StdFun() : Obj(std::nullopt) {}
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& /*gca*/) const override {
            return clone();
        }
        [[nodiscard]] std::unique_ptr<Obj> clone() const override { return std::make_unique<T>(); }
    };

    class StdInput : public StdFun<StdInput> {
      public:
        void execute(Array& gca, std::istream& in, std::ostream& out) override;
    };

    class StdOutput : public StdFun<StdOutput> {
      public:
        void execute(Array& gca, std::istream& in, std::ostream& out) override;
    };

    class StdDecompose : public StdFun<StdDecompose> {
      public:
        void execute(Array& gca, std::istream& in, std::ostream& out) override;
    };

    class Runtime {
      private:
        // glocal circular array
        Array m_gca;
        Array m_code;

      public:
        explicit Runtime(ast::Array&& code);
        void run(std::istream& in, std::ostream& out, std::ostream& err);
    };
} // namespace runtime
