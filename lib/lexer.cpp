#include "lexer.hpp"
#include "number.hpp"
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <tl/expected.hpp>

namespace partial {
    class Token;
    using NextState =
        std::optional<std::pair<std::unique_ptr<Token>, std::optional<::token::Kind>>>;

    class Token {
      public:
        Token() = default;
        Token(const Token&) = default;
        Token(Token&&) = default;
        Token& operator=(const Token&) = default;
        Token& operator=(Token&&) = default;
        virtual NextState read_char(char);
        virtual ~Token() = default;
    };

    class WhiteSpace : public Token {
      public:
        NextState read_char(char c) override;
    };
    class OpenBracket : public Token {
      private:
        int m_count{};

      public:
        NextState read_char(char c) override;
    };
    class CloseBracket : public Token {
      private:
        int m_count{};

      public:
        NextState read_char(char c) override;
    };
    class Comment : public Token {
      private:
        std::stringstream m_content{};

      public:
        NextState read_char(char c) override;
    };
    class Assign : public Token {
      public:
        NextState read_char(char c) override;
    };
    class Number : public Token {
      private:
        std::stringstream m_content{};

      public:
        explicit Number(char c) : m_content{std::string{c}} {}
        NextState read_char(char c) override;
    };
    class BoolAnd : public Token {
      public:
        NextState read_char(char c) override;
    };
    class BoolOr : public Token {
      public:
        NextState read_char(char c) override;
    };
    class NotEqual : public Token {
      public:
        NextState read_char(char c) override;
    };
    class Smaller : public Token {
      public:
        NextState read_char(char c) override;
    };
    class Greater : public Token {
      public:
        NextState read_char(char c) override;
    };

    NextState WhiteSpace::read_char(char c) {
        if (isspace(c)) {
            return std::nullopt;
        } else {
            switch (c) {
            case '(':
                return std::make_pair(std::make_unique<OpenBracket>(), std::nullopt);
            case ')':
                return std::make_pair(std::make_unique<CloseBracket>(), std::nullopt);
            case '#':
                return std::make_pair(std::make_unique<Comment>(), std::nullopt);
            case '=':
                return std::make_pair(std::make_unique<Assign>(), std::nullopt);
            default:
                if (::Number::is_in_char_set(c)) {
                    return std::make_pair(std::make_unique<Number>(c), std::nullopt);
                } else {
                }
            }
        }
    }
    // using Kind = std::variant<std::monostate, OpenBracket, CloseBracket,
    //                           Comment, Assign, Number>;
} // namespace partial

tl::expected<std::vector<token::Token>, std::string> lex(const std::string& src_code) {
    std::vector<token::Token> tokens{};
    std::unique_ptr<partial::Token> partial{std::make_unique<partial::WhiteSpace>()};
    diagnostic::Range range{};
    for (const char& c : src_code) {
        if (c == '\n') {
        }
        auto next_state = partial->read_char(c);
        if (next_state) {
            partial = std::move(next_state->first);
            if (next_state->second) {
                tokens.push_back(token::Token{.kind{*next_state->second}, .range{}});
            }
        }
    }
}
