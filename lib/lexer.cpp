#include "lexer.hpp"
#include "number.hpp"
#include <memory>
#include <sstream>
#include <string>
#include <tl/expected.hpp>

namespace partial {
    class Token;
    using NextState =
        tl::expected<std::optional<std::pair<std::unique_ptr<Token>, std::optional<::token::Kind>>>,
                     std::string>;

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
        int m_count{1};

      public:
        NextState read_char(char c) override;
    };

    class CloseBracket : public Token {
      private:
        int m_count{1};

      public:
        NextState read_char(char c) override;
    };

    class Comment : public Token {
      private:
        std::stringstream m_content;

      public:
        NextState read_char(char c) override;
    };

    class Number : public Token {
      private:
        std::stringstream m_content;

      public:
        explicit Number(char c) : m_content{std::string{c}} {}
        NextState read_char(char c) override;
    };

    class Operator : public Token {
      private:
        std::stringstream m_content;

      public:
        explicit Operator(char c) : m_content{std::string{c}} {}
        NextState read_char(char c) override;
    };

    inline std::unique_ptr<Token> new_partial(char c) {
        if (std::isspace(c) != 0) {
            return std::make_unique<WhiteSpace>();
        } else {
            switch (c) {
            case '(':
                return std::make_unique<OpenBracket>();
            case ')':
                return std::make_unique<CloseBracket>();
            case '#':
                return std::make_unique<Comment>();
            default:
                if (number::is_in_char_set(c)) {
                    return std::make_unique<Number>(c);
                } else {
                    return std::make_unique<Operator>(c);
                }
            }
        }
    }

    NextState WhiteSpace::read_char(char c) {
        if (std::isspace(c) != 0) {
            return std::nullopt;
        } else {
            return std::make_pair(new_partial(c), std::nullopt);
        }
    }

    NextState OpenBracket::read_char(char c) {
        if (c == '(') {
            m_count++;
            return std::nullopt;
        } else {
            switch (m_count) {
            case 1:
                return std::make_pair(new_partial(c), token::OpenBracket{});
            case 2:
                return std::make_pair(new_partial(c), token::OpenBracket2{});
            default:
                std::stringstream err_msg{};
                err_msg << "\"" << std::string(m_count, '(') << "\""
                        << " is too many \'(\' in a row. Try splitting them up with space.";
                return tl::unexpected(err_msg.str());
            }
        }
    }

    NextState CloseBracket::read_char(char c) {
        if (c == ')') {
            m_count++;
            return std::nullopt;
        } else {
            switch (m_count) {
            case 1:
                return std::make_pair(new_partial(c), token::CloseBracket{});
            case 2:
                return std::make_pair(new_partial(c), token::CloseBracket2{});
            default:
                std::stringstream err_msg{};
                err_msg << "\"" << std::string(m_count, ')') << "\""
                        << " is too many \')\' in a row. Try splitting them up with space.";
                return tl::unexpected(err_msg.str());
            }
        }
    }

    NextState Comment::read_char(char c) {
        if (c == '\n') {
            return std::make_pair(std::make_unique<WhiteSpace>(),
                                  token::Comment{.content{m_content.str()}});
        } else {
            m_content << c;
            return std::nullopt;
        }
    }

    NextState Number::read_char(char c) {
        if (number::is_in_char_set(c)) {
            m_content << c;
            return std::nullopt;
        } else {
            return std::make_pair(new_partial(c), token::Number{.value{"hehe"}});
        }
    }

    // NextState Operator::
} // namespace partial

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> lex(const std::string& src_code) {
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
