#pragma once

#include "diagnostic.hpp"
#include "number.hpp"
#include <string>
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
    struct Plus {};
    struct Minus {};
    struct Multiply {};
    struct Divide {};
    struct Remainder {};
    struct BoolAnd {};
    struct BoolOr {};
    struct Equal {};
    struct NotEqual {};
    struct Smaller {};
    struct SmallerOrEqual {};
    struct Greater {};
    struct GreaterOrEqual {};

    using Kind = std::variant<OpenBracket,    //
                              CloseBracket,   //
                              Semicolon,      //
                              OpenBracket2,   //
                              CloseBracket2,  //
                              Comment,        //
                              Number,         //
                              Assign,         //
                              Plus,           //
                              Minus,          //
                              Multiply,       //
                              Divide,         //
                              Remainder,      //
                              BoolAnd,        //
                              BoolOr,         //
                              Equal,          //
                              NotEqual,       //
                              Smaller,        //
                              SmallerOrEqual, //
                              Greater,        //
                              GreaterOrEqual>;

    struct Token {
        diagnostic::Range range;
        Kind kind;
    };

    std::string to_string(Token t);
} // namespace token

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> lex(std::string_view src_code);
