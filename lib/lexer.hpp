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
        std::string content;
    };
    struct Number {
        number::Value value;
    };
    struct Assign {};
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
                              Number, Assign, Plus, Minus, Multiply, Divide, BoolAnd, BoolOr, Equal,
                              NotEqual, Smaller, SmallerOrEqual, Greater, GreaterOrEqual>;

    struct Token {
        diagnostic::Range range;
        Kind kind;
    };
} // namespace token

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> lex(const std::string& src_code);
