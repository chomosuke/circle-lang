#pragma once

#include "number.hpp"
#include <string>
#include <tl/expected.hpp>
#include <variant>

namespace Token {
    struct OpenBracket {};
    struct OpenBracket2 {};
    struct CloseBracket {};
    struct CloseBracket2 {};
    struct Comment {
        std::string content{};
    };
    struct Assign {};
    struct NumberT {
        Number value{};
    };

    using Kind = std::variant<OpenBracket, OpenBracket2, CloseBracket,
                              CloseBracket2, Comment, Assign, NumberT>;
} // namespace Token

tl::expected<std::vector<Token::Kind>, std::string>
lex(const std::string& src_code);
