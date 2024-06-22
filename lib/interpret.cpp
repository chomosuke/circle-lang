#include "lexer.hpp"
#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& in, std::ostream& out,
               std::ostream& err) {
    out << "Hello interpreter" << std::endl;

    lex("hehe");
}
