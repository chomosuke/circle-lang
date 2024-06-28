#pragma once

#include "vendor/BigInt.hpp"
#include <string_view>
#include <vector>

namespace number {
    enum Operator {
        plus,
        minus,
        multiply,
        divide,
        remainder,
        bool_and,
        bool_or,
        equal,
        not_equal,
        smaller,
        smaller_or_equal,
        greater,
        greater_or_equal,
    };

    bool is_in_char_set(char c);

    constexpr int LETTER_BASE{256};

    class Value {
      private:
        std::vector<BigInt> m_numerator;
        std::vector<BigInt> m_denominator;

      public:
        explicit Value(std::string_view letters);
        explicit Value(const BigInt& number);

        const std::vector<BigInt>& get_numerator();
        const std::vector<BigInt>& get_denominator();
        std::optional<std::string> to_letters();
    };
} // namespace number
