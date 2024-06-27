#include "lib/lexer.cpp"
#include "test/sample-programs.hpp"
#include <gtest/gtest.h>

std::string tokens_to_string(const std::vector<token::Token>& ts) {
    std::stringstream ss{};
    for (const auto& t : ts) {
        ss << token::to_string(t) << " ";
    }
    return ss.str();
}

TEST(Lex, HelloWorld) {
    EXPECT_EQ(tokens_to_string(lex(R"()").value()), "");
    EXPECT_EQ(
        tokens_to_string(lex(sample_programs::HELLO_WORLD).value()),
        "( S ) ; ( V ) = V_a ; # ((array)) -> len ( F_len ) = (( ( ( V ) + {0 1}{1} * {0 1}{1} ) ; "
        "( V ) = ( V ) + {0 1}{1} * {0 1}{1} ; ( ( V ) ) ( Array ) = ( ( V ) ) ( {0 0}{1} ) ; # "
        "loop ( ( V ) ) ( I ) = {0 1}{1} ; (( ( ( V ) ) ( Found_len ) ; ( ( V ) ) ( 1Temp ) = ( ( "
        "V ) ) ( Array ) ( {0 0}{1} ) ; ( ( V ) ) ( 2Temp ) = ( ( V ) ) ( Array ) ( ( ( V ) ) ( I "
        ") ) ; ( ( V ) ) ( Array ) ( {0 0}{1} ) = {0 0}{1} ; ( ( V ) ) ( Array ) ( ( ( V ) ) ( I ) "
        ") = {0 1}{1} ; ( ( V ) ) ( Found_len ) = ( ( V ) ) ( Array ) ( {0 0}{1} ) = {0 1}{1} ; ( "
        "( V ) ) ( Array ) ( {0 0}{1} ) = ( ( V ) ) ( 1Temp ) ; ( ( V ) ) ( Array ) ( ( ( V ) ) ( "
        "I ) ) = ( ( V ) ) ( 2Temp ) ; ( ( V ) ) ( I ) = ( ( V ) ) ( I ) + {0 1}{1} ; )) ; ( ( V ) "
        ") ( I ) = ( ( V ) ) ( I ) - {0 1}{1} ; ( R ) = ( ( V ) ) ( I ) ; ( ( V ) ) = {0 0}{1} ; ( "
        "V ) = ( V ) - {0 1}{1} * {0 1}{1} ; )) ; # ((str)) ( F_print_str ) = (( ( ( V ) + {0 "
        "1}{1} * {0 1}{1} ) ; ( V ) = ( V ) + {0 1}{1} * {0 1}{1} ; ( ( V ) ) ( Str ) = ( ( V ) ) "
        "( {0 0}{1} ) ; ( ( V ) + {0 1}{1} * {0 1}{1} ) = (( ( ( V ) ) ( Str ) )) ; ( F_len ) ; ( "
        "( V ) ) ( Len ) = ( R ) ; # loop ( ( ( V ) ) ( I ) ) = {0 0}{1} ; (( ( ( V ) ) ( I ) < ( "
        "( V ) ) ( Len ) ; ( _charstd_output ) = ( ( V ) ) ( Str ) ( ( ( V ) ) ( I ) ) ; ( "
        "_outputstd ) ; ( ( V ) ) ( I ) = ( ( V ) ) ( I ) + {0 1}{1} ; )) ; ( _charstd_output ) = "
        "{0 10}{1} ; ( _outputstd ) ; ( ( V ) ) = {0 0}{1} ; ( V ) = ( V ) - {0 1}{1} * {0 1}{1} ; "
        ")) ; # Hello world\\n ( ( V ) + {0 1}{1} * {0 1}{1} ) = (( (( {0 72}{1} ; {0 101}{1} ; {0 "
        "108}{1} ; {0 108}{1} ; {0 111}{1} ; {0 32}{1} ; {0 87}{1} ; {0 111}{1} ; {0 114}{1} ; {0 "
        "108}{1} ; {0 100}{1} ; {0 33}{1} ; {0 10}{1} )) )) ; ( F_print_str ) ; ( S ) = {0 0}{1} "
        "; ");
    // EXPECT_EQ((diagnostic::Range{{{0}, {0}}, {{0}, {0}}}),
    //           (diagnostic::Range{{{0}, {0}}, {{0}), {0}}}));
}
