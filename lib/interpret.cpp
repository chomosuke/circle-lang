#include "interpret.hpp"

#include "config.hpp"
#include "diagnostic.hpp"
#include "parser.hpp"
#include "runtime.hpp"

#include <ostream>
#include <string>

void interpret(const std::string& src_code, std::istream& in, std::ostream& out, std::ostream& err,
               const Config& config) {
    auto diags = diag::Diags();
    auto parsed = parse(src_code, diags);
    if (!diags.empty()) {
        err << diags.to_string() << '\n';
        if (diags.has_fatal()) {
            return;
        }
    }

    if (config.debug) {
        auto runtime = runtime::Runtime<true>(std::move(*parsed), src_code);
        runtime.run(in, out, err);
    } else {
        auto runtime = runtime::Runtime<false>(std::move(*parsed), src_code);
        runtime.run(in, out, err);
    }
}
