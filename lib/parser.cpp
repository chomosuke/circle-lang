#include "parser.hpp"
#include "lib/lexer.hpp"
#include <string_view>
#include <tl/expected.hpp>

tl::expected<std::vector<token::Token>, diagnostic::Diagnostic> parse(std::string_view src_code) {

}
