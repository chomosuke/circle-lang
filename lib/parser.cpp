#include "parser.hpp"
#include "diagnostic.hpp"
#include "lexer.hpp"
#include <tl/expected.hpp>

tl::expected<ast_node::Node<ast_node::Any>, diagnostic::Diagnostic>
parse(std::span<token::Token> tokens) {
	
}
