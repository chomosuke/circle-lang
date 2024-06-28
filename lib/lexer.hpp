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
    struct Operator {
        number::Operator kind;
    };

    using Kind = std::variant<OpenBracket,    //
                              CloseBracket,   //
                              Semicolon,      //
                              OpenBracket2,   //
                              CloseBracket2,  //
                              Comment,        //
                              Number,         //
                              Assign,         //
                              Operator>;

    struct Token {
        diagnostic::Range range;
        Kind kind;
    };
} // namespace token

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> lex(std::string_view src_code);
