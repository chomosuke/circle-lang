#include "format.hpp"

#include "diagnostic.hpp"
#include "lexer.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>
#include <string>

std::string format(std::optional<std::vector<token::Token>>&& lexed) {
    std::stringstream ss{};
    int indent = 0;
    for () {
        std::visit(
            [&]<typename T>(T&& t) -> bool {
                static_assert(std::is_same_v<T, std::decay_t<T>>);
                if constexpr (std::is_same_v<T, token::OpenBracket2>) {
                    ss << "((\n";
                    indent++;
                } else if constexpr (std::is_same_v<T, token::CloseBracket2>) {
                    ss << '\n';
                    indent--;
                    print_indent(ss, indent);
                    ss << "))\n";
                } else if constexpr (std::is_same_v<T, token::Semicolon>) {
                    ss << '\n';
                    print_indent(ss, indent);
                    ss << "; ";
                } else if constexpr (std::is_same_v<T, token::Comment>) {
                    print_indent(ss, indent);
                } else if constexpr (std::is_same_v<T, token::OpenBracket> ||
                                     std::is_same_v<T, token::CloseBracket> ||
                                     std::is_same_v<T, token::Number> ||
                                     std::is_same_v<T, token::Assign> ||
                                     std::is_same_v<T, token::OperatorBinary> ||
                                     std::is_same_v<T, token::OperatorUnary>) {
                } else {
                    static_assert(false, "Not exhaustive");
                }
                return false;
            },
            std::move(token.t));
    }
}

std::string format(const std::string& src_code, std::ostream& err) {
    auto diags = diag::Diags();
    auto lexed = lex(src_code, diags);
    if (!diags.empty()) {
        err << diags.to_string() << '\n';
    }
    if (!lexed) {
        return src_code;
    }
    return format(std::move(lexed));
}
