#include "parser.hpp"
#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& /*in*/, std::ostream& /*out*/,
               std::ostream& err) {
    diag::Diags diags;
    auto parsed = parse(src_code, diags);
    if (!diags.empty()) {
        err << diags.to_string() << '\n';
        if (diags.has_fatal()) {
            return;
        }
    }
    
}
