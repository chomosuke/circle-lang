#include "lexer.hpp"
#include "parser.hpp"
#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& /*in*/, std::ostream& out,
               std::ostream& err) {
    auto lexed = lex(src_code);
    if (!lexed) {
        const auto& diagnostic = lexed.error();
        err << diagnostic.to_string() << '\n';
        return;
    }
    auto& tokens = lexed.value();
    parse(tokens);
}
