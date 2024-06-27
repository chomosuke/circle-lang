#pragma once

#include "vendor/BigInt.hpp"
#include <string_view>
#include <vector>

namespace number {
    bool is_in_char_set(char c);

    constexpr int LETTER_BASE{256};

    class Value {
      private:
        std::vector<BigInt> m_numerator;
        std::vector<BigInt> m_denominator;

      public:
        explicit Value(std::string_view letters);

        const std::vector<BigInt>& get_numerator();
        const std::vector<BigInt>& get_denominator();
        std::optional<std::string> to_letters();
    };
} // namespace number
