#include "parser.hpp"
#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& /*in*/, std::ostream& /*out*/,
               std::ostream& err) {
    diag::Diags diags;
    auto parsed = parse(src_code, diags);
    if (!parsed) {
        err << diags.to_string() << '\n';
        return;
    }
}
