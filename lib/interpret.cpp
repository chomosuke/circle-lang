#include "interpret.hpp"

#include "diagnostic.hpp"
#include "parser.hpp"
#include "runtime.hpp"

#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& in, std::ostream& out,
               std::ostream& err) {
    auto diags = diag::Diags();
    auto parsed = parse(src_code, diags);
    if (!diags.empty()) {
        err << diags.to_string() << '\n';
        if (diags.has_fatal()) {
            return;
        }
    }

    auto runtime = runtime::Runtime(std::move(*parsed));
    runtime.run(in, out, err);
}
