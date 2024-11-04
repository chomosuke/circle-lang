#pragma once

#include "diagnostic.hpp"
#include "macros.hpp"
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
        NON_COPIABLE(Comment)

        std::string content;

        explicit Comment(std::string&& content);
    };
    struct Number {
        NON_COPIABLE(Number)

        number::Value value;

        explicit Number(number::Value&& value);
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

    using Token = diag::WithInfo<Kind>;
} // namespace token

std::optional<std::vector<token::Token>> lex(std::string_view src_code, diag::Diags &diags);
