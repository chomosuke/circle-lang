#include "lib/lexer.hpp"
#include <string>

namespace token {
    template <typename T> inline std::string to_string(const T& t) {
        if constexpr (std::is_same_v<T, OpenBracket>) {
            return "( ";
        } else if constexpr (std::is_same_v<T, CloseBracket>) {
            return " )";
        } else if constexpr (std::is_same_v<T, Semicolon>) {
            return "; ";
        } else if constexpr (std::is_same_v<T, OpenBracket2>) {
            return "(( ";
        } else if constexpr (std::is_same_v<T, CloseBracket2>) {
            return " ))";
        } else if constexpr (std::is_same_v<T, Comment>) {
            return std::format("#{}", t.content);
        } else if constexpr (std::is_same_v<T, Number>) {
            return diag::to_string(t.value);
        } else if constexpr (std::is_same_v<T, Assign>) {
            return " := ";
        } else if constexpr (std::is_same_v<T, OperatorBinary>) {
            switch (t.kind) {
            case number::op::plus:
                return " + ";
            case number::op::minus:
                return " - ";
            case number::op::multiply:
                return " * ";
            case number::op::divide:
                return " / ";
            case number::op::remainder:
                return " % ";
            case number::op::bool_and:
                return " && ";
            case number::op::bool_or:
                return " || ";
            case number::op::equal:
                return " = ";
            case number::op::not_equal:
                return " != ";
            case number::op::smaller:
                return " < ";
            case number::op::smaller_or_equal:
                return " <= ";
            case number::op::greater:
                return " > ";
            case number::op::greater_or_equal:
                return " >= ";
            }
        } else if constexpr (std::is_same_v<T, OperatorUnary>) {
            switch (t.kind) {
            case number::op::bool_not:
                return "!";
            }
        } else {
            static_assert(false, "Not exhaustive");
        }
    }
} // namespace token
