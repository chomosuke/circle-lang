#include "number.hpp"

namespace number {
    bool is_in_char_set(char c) {
        return (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ||
                c == '_');
    }

    Value::Value(std::string letters) {
        
    }
} // namespace number
