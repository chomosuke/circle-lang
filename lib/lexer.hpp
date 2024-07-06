#pragma once

#include "diagnostic.hpp"
#include "number.hpp"
#include <string>
#include <string_view>
#include <tl/expected.hpp>
#include <variant>

namespace token {
    struct OpenBracket {};
    struct CloseBracket {};
    struct Semicolon {};
    struct OpenBracket2 {};
    struct CloseBracket2 {};
    struct Comment {
        std::string content;
    };
    struct Number {
        number::Value value;
    };
    struct Assign {};
    struct OperatorBinary {
        number::op::Binary kind;
    };
    struct OperatorUnary {
        number::op::Unary kind;
    };

    using Kind = std::variant< //
        OpenBracket,           //
        CloseBracket,          //
        Semicolon,             //
        OpenBracket2,          //
        CloseBracket2,         //
        Comment,               //
        Number,                //
        Assign,                //
        OperatorBinary,        //
        OperatorUnary>;

    using Token = diagnostic::WithInfo<Kind>;
} // namespace token

diagnostic::Expected<std::vector<token::Token>> lex(std::string_view src_code);
