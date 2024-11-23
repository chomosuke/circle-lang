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
        Obj(Obj&&) = delete;
        Obj& operator=(const Obj&) = delete;
        Obj& operator=(Obj&&) = delete;
        virtual ~Obj() = default;

        explicit Obj(std::optional<diag::Range> range);

        [[nodiscard]] std::optional<diag::Range> get_range() const;

        virtual void execute(Array& gca) = 0;
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

        void insert(std::vector<number::Value>&& indices, std::unique_ptr<Obj> v);
        std::unique_ptr<Obj> index(const number::Value& i) const;

        void execute(Array& gca) override;
        std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        std::unique_ptr<Obj> clone() const override;
    };

    class Index : public Obj {
      private:
        std::optional<std::unique_ptr<Obj>> m_subject;
        std::unique_ptr<Obj> m_index;
        Index(std::optional<std::unique_ptr<Obj>>&& subject, std::unique_ptr<Obj>&& index, std::optional<diag::Range> range);

      public:
        Index(ast::Index&& node, diag::Range range);

        [[nodiscard]] std::optional<std::vector<number::Value>>
        get_gca_location(const Array& gca) const;

        void execute(Array& gca) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class Assign : public Obj {
      private:
        std::unique_ptr<Index> m_lhs;
        std::unique_ptr<Obj> m_rhs;

      public:
        Assign(ast::Assign&& node, diag::Range range);

        void execute(Array& gca) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class OperatorBinary : public Obj {
      private:
        number::op::Binary m_kind;
        std::unique_ptr<Obj> m_lhs;
        std::unique_ptr<Obj> m_rhs;

      public:
        OperatorBinary(ast::OperatorBinary&& node, diag::Range range);

        void execute(Array& gca) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
    };

    class OperatorUnary : public Obj {
      private:
        number::op::Unary m_kind;
        std::unique_ptr<Obj> m_rhs;

      public:
        OperatorUnary(ast::OperatorUnary&& node, diag::Range range);

        void execute(Array& gca) override;
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

        void execute(Array& gca) override;
        [[nodiscard]] std::unique_ptr<Obj> evaluate(const Array& gca) const override;
        [[nodiscard]] std::unique_ptr<Obj> clone() const override;
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
