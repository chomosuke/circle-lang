#pragma once

#include "vendor/BigInt.hpp"
#include <string>
#include <vector>

namespace number {
    bool is_in_char_set(char c);

    class Value {
      private:
        std::vector<BigInt> m_numerator;
        std::vector<BigInt> m_denominator;

      public:
        explicit Value(std::string letters);
    };
} // namespace number
