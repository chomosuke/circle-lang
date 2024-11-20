#pragma once

#include "macros.hpp"
#include "vendor/BigInt.hpp"
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

namespace number {
    namespace op {
        enum Binary : std::uint8_t {
            plus,
            minus,
            multiply,
            divide,
            bool_and,
            bool_or,
            equal,
            not_equal,
            smaller,
            smaller_or_equal,
            greater,
            greater_or_equal,
        };
        enum Unary : std::uint8_t {
            bool_not,
        };
    } // namespace op

    bool is_in_char_set(char c);

    constexpr int LETTER_BASE{256};

    class Value {
      private:
        // Invariant: The fraction will always be simplified with simplify().
        std::vector<BigInt> m_numerator;
        std::vector<BigInt> m_denominator;

      public:
        NON_COPIABLE(Value)

        explicit Value(std::string_view letters);
        explicit Value(const BigInt& number);
        explicit Value(const std::vector<BigInt>& num, const std::vector<BigInt>& den);
        [[nodiscard]] Value clone() const;

        [[nodiscard]] const std::vector<BigInt>& get_numerator() const;
        [[nodiscard]] const std::vector<BigInt>& get_denominator() const;
        [[nodiscard]] std::optional<std::string> to_letters() const;
        [[nodiscard]] std::string to_string() const;
        [[nodiscard]] bool to_bool() const;
    };

    [[nodiscard]] Value operator+(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator-(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator*(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator/(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator&&(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator||(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator==(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator!=(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator<(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator<=(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator>(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator>=(const Value& lhs, const Value& rhs);
    [[nodiscard]] Value operator!(const Value& lhs);

    class Index {
        // An index of a circular array
      private:
        Value m_value;
        int m_length;
        std::size_t m_hash;

      public:
        Index(Value&& value, int length);
        [[nodiscard]] Index clone() const;

        bool operator==(const Index& rhs) const;
        [[nodiscard]] std::size_t hash() const;
    };
} // namespace number

template <> struct std::hash<number::Index> {
    std::size_t operator()(const number::Index& i) const noexcept;
};
