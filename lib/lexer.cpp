#include "lexer.hpp"
#include <sstream>
#include <string>
#include <tl/expected.hpp>
#include <type_traits>

namespace Partial {
    struct OpenBracket {
        int count{};
    };
    struct CloseBracket {
        int count{};
    };
    struct Comment {
        std::stringstream content{};
    };
    struct Assign {};
    struct Number {
        std::stringstream content{};
    };

    using Kind = std::variant<std::monostate, OpenBracket, CloseBracket,
                              Comment, Assign, Number>;
} // namespace Partial

tl::expected<std::vector<Token::Kind>, std::string>
lex(const std::string& src_code) {
    Partial::Kind partial{std::monostate{}};
    for (const char& c : src_code) {
        std::visit(
            []<typename T>(T& p) {
                if constexpr (std::is_same_v<T, std::monostate>) {
                } else if constexpr (std::is_same_v<T, Partial::OpenBracket>) {
                } else if constexpr (std::is_same_v<T, Partial::CloseBracket>) {
                } else if constexpr (std::is_same_v<T, Partial::Comment>) {
                } else if constexpr (std::is_same_v<T, Partial::Assign>) {
                } else if constexpr (std::is_same_v<T, Partial::Number>) {
                } else {
                    static_assert(false, "Not exhaustive");
                }
            },
            partial);
    }
}
