#include "lexer.hpp"

#include "diagnostic.hpp"
#include "macros.hpp"
#include "number.hpp"

#include <array>
#include <climits>
#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <tl/expected.hpp>

namespace partial {
    class Token;

    using NextState = std::pair<std::unique_ptr<Token>, std::optional<::token::Kind>>;
    using ReadCharResult = tl::expected<std::optional<NextState>, std::string>;

    class Token {
      public:
        Token() = default;
        Token(const Token&) = default;
        Token(Token&&) = default;
        Token& operator=(const Token&) = default;
        Token& operator=(Token&&) = default;
        virtual ReadCharResult read_char(char) = 0;
        virtual ~Token() = default;
    };

    class WhiteSpace : public Token {
      public:
        ReadCharResult read_char(char c) override;
    };

    class OpenBracket : public Token {
      private:
        int m_count{1};

      public:
        ReadCharResult read_char(char c) override;
    };

    class CloseBracket : public Token {
      private:
        int m_count{1};

      public:
        ReadCharResult read_char(char c) override;
    };

    class Semicolon : public Token {
      public:
        ReadCharResult read_char(char c) override;
    };

    class Comment : public Token {
      private:
        std::stringstream m_content;

      public:
        NON_COPIABLE(Comment)

        Comment() = default;
        ReadCharResult read_char(char c) override;
    };

    class Number : public Token {
      private:
        std::stringstream m_content;

      public:
        NON_COPIABLE(Number)

        explicit Number(char c) { m_content << c; }
        ReadCharResult read_char(char c) override;
    };

    class Operator : public Token {
      private:
        std::stringstream m_content;

      public:
        NON_COPIABLE(Operator);

        explicit Operator(char c) { m_content << c; }
        ReadCharResult read_char(char c) override;
    };

    consteval std::array<bool, CHAR_MAX> operator_char_set() {
        std::array<bool, CHAR_MAX> lookup{};
        std::string char_set{"+-*/&|=!<>:"};
        for (auto c : char_set) {
            lookup[c] = true; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
        }
        return lookup;
    }
    bool is_in_operator_char_set(char c) {
        return operator_char_set()[c]; // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
    }

    inline tl::expected<std::unique_ptr<Token>, std::string> new_partial(char c) {
        if (std::isspace(c) != 0) {
            return std::make_unique<WhiteSpace>();
        } else {
            switch (c) {
            case '(':
                return std::make_unique<OpenBracket>();
            case ')':
                return std::make_unique<CloseBracket>();
            case ';':
                return std::make_unique<Semicolon>();
            case '#':
                return std::make_unique<Comment>();
            default:
                if (number::is_in_char_set(c)) {
                    return std::make_unique<Number>(c);
                } else if (is_in_operator_char_set(c)) {
                    return std::make_unique<Operator>(c);
                } else {
                    return tl::unexpected(std::format("\'{}\' is not a valid character.", c));
                }
            }
        }
    }

    inline ReadCharResult state_with_new_partial(char c, std::optional<token::Kind>&& token) {
        return new_partial(c).map([token{std::move(token)}](std::unique_ptr<Token> p) mutable {
            return std::make_optional(std::make_pair(std::move(p), std::move(token)));
        });
    }

    ReadCharResult WhiteSpace::read_char(char c) {
        if (std::isspace(c) != 0) {
            return std::nullopt;
        } else {
            return state_with_new_partial(c, std::nullopt);
        }
    }

    ReadCharResult OpenBracket::read_char(char c) {
        if (c == '(') {
            m_count++;
            return std::nullopt;
        } else {
            switch (m_count) {
            case 1:
                return state_with_new_partial(c, token::OpenBracket{});
            case 2:
                return state_with_new_partial(c, token::OpenBracket2{});
            default:
                return tl::unexpected(std::format(
                    R"("{}" is too many '(' in a row. Try splitting them up with space.)",
                    std::string(m_count, '(')));
            }
        }
    }

    ReadCharResult CloseBracket::read_char(char c) {
        if (c == ')') {
            m_count++;
            return std::nullopt;
        } else {
            switch (m_count) {
            case 1:
                return state_with_new_partial(c, token::CloseBracket{});
            case 2:
                return state_with_new_partial(c, token::CloseBracket2{});
            default:
                return tl::unexpected(std::format(
                    R"("{}" is too many ')' in a row. Try splitting them up with space.)",
                    std::string(m_count, ')')));
            }
        }
    }

    ReadCharResult Semicolon::read_char(char c) {
        return state_with_new_partial(c, token::Semicolon{});
    }

    ReadCharResult Comment::read_char(char c) {
        if (c == '\n') {
            return std::make_optional(
                std::make_pair(std::make_unique<WhiteSpace>(), token::Comment(m_content.str())));
        } else {
            m_content << c;
            return std::nullopt;
        }
    }

    ReadCharResult Number::read_char(char c) {
        if (number::is_in_char_set(c)) {
            m_content << c;
            return std::nullopt;
        } else {
            try {
                return state_with_new_partial(
                    c, token::Number(number::Value(BigInt(m_content.str()))));
            } catch (std::invalid_argument&) {
                return state_with_new_partial(
                    c, token::Number(number::Value(std::string_view(m_content.str()))));
            }
        }
    }

    ReadCharResult
    Operator::read_char(char c) { // NOLINT(readability-function-cognitive-complexity)
        if (is_in_operator_char_set(c)) {
            m_content << c;
            return std::nullopt;
        } else {
            token::Kind op;
            auto content = m_content.str();
            if (content == ":=") {
                return state_with_new_partial(c, token::Assign{});
            }
            if (content == "!") {
                return state_with_new_partial(c, token::OperatorUnary{.kind{number::op::bool_not}});
            }
            if (content == "+") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::plus}});
            }
            if (content == "-") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::minus}});
            }
            if (content == "*") {
                return state_with_new_partial(c,
                                              token::OperatorBinary{.kind{number::op::multiply}});
            }
            if (content == "/") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::divide}});
            }
            if (content == "&&") {
                return state_with_new_partial(c,
                                              token::OperatorBinary{.kind{number::op::bool_and}});
            }
            if (content == "||") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::bool_or}});
            }
            if (content == "=") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::equal}});
            }
            if (content == "!=") {
                return state_with_new_partial(c,
                                              token::OperatorBinary{.kind{number::op::not_equal}});
            }
            if (content == "<") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::smaller}});
            }
            if (content == "<=") {
                return state_with_new_partial(
                    c, token::OperatorBinary{.kind{number::op::smaller_or_equal}});
            }
            if (content == ">") {
                return state_with_new_partial(c, token::OperatorBinary{.kind{number::op::greater}});
            }
            if (content == ">=") {
                return state_with_new_partial(
                    c, token::OperatorBinary{.kind{number::op::greater_or_equal}});
            }
            return tl::unexpected(std::format("\"{}\" is not a valid operator.", m_content.str()));
        }
    }
} // namespace partial

namespace token {
    Comment::Comment(std::string&& content) : content{std::move(content)} {}

    Number::Number(number::Value&& value) : value{std::move(value)} {}
} // namespace token

std::optional<std::vector<token::Token>> lex(std::string_view src_code, diag::Diags& diags) {
    std::vector<token::Token> tokens{};
    std::unique_ptr<partial::Token> partial{std::make_unique<partial::WhiteSpace>()};
    diag::Range range{.start{.line{0}, .column{0}}, .end{.line{0}, .column{0}}};
    auto read_char = [&](char c) {
        auto maybe_next_state = partial->read_char(c);
        if (maybe_next_state) {
            auto next_state = std::move(*maybe_next_state);
            if (next_state) {
                partial = std::move(next_state->first);
                if (next_state->second) {
                    tokens.push_back(
                        token::Token{.range{range}, .t{std::move(*next_state->second)}});
                }
                range.start = range.end;
            }
        } else {
            diags.insert(diag::Diagnostic{
                .level{diag::error}, .range{range}, .message{maybe_next_state.error()}});
        }
    };
    for (const char& c : src_code) {
        read_char(c);
        if (diags.has_fatal()) {
            return std::nullopt;
        }
        if (c == '\n') {
            range.end.column = 0;
            range.end.line++;
        } else {
            range.end.column++;
        }
    }
    if (!src_code.empty() && std::isspace(src_code.back()) == 0) {
        read_char('\n');
    }
    return tokens;
}
