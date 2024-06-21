#pragma once

#include "diagnostic.hpp"
#include "number.hpp"
#include <string>
#include <tl/expected.hpp>
#include <variant>

namespace token {
    struct OpenBracket {};
    struct OpenBracket2 {};
    struct CloseBracket {};
    struct CloseBracket2 {};
    struct Comment {
        std::string content{};
    };
    struct Assign {};
    struct Number {
        number::Value value{};
    };
    struct Plus {};
    struct Minus {};
    struct Multiply {};
    struct Divide {};
    struct BoolAnd {};
    struct BoolOr {};
    struct Equal {};
    struct NotEqual {};
    struct Smaller {};
    struct SmallerOrEqual {};
    struct Greater {};
    struct GreaterOrEqual {};

    using Kind = std::variant<OpenBracket, OpenBracket2, CloseBracket, CloseBracket2, Comment,
                              Assign, Number, Plus, Minus, Multiply, Divide, BoolAnd, BoolOr, Equal,
                              NotEqual, Smaller, SmallerOrEqual, Greater, GreaterOrEqual>;

    struct Token {
        Kind kind{};
        diagnostic::Range range{};
    };
} // namespace token

tl::expected<std::vector<token::Token>, std::string> lex(const std::string& src_code);
