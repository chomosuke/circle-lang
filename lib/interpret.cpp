#include "lexer.hpp"
#include "parser.hpp"
#include <ostream>
#include <string>

// NOLINTNEXTLINE(clang-diagnostic-unused-parameter, misc-unused-parameters)
void interpret(const std::string& src_code, std::istream& /*in*/, std::ostream& out,
               std::ostream& err) {
    diag::Diags diags;
    auto lexed = lex(src_code, diags);
    if (!lexed) {
        err << diag::to_string(diags) << '\n';
        return;
    }
    auto& tokens = lexed.value();
    auto parsed = parse(tokens, diags);
    if (!parsed) {
        err << diag::to_string(diags) << '\n';
        return;
    }
}
