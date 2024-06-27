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
        "( S ) ; ( V ) = 0V_ ; # ((array)) -> len ( F_len ) = (( ( ( V ) + 1 * 1 ) ; ( V ) = ( V ) "
        "+ 1 * 1 ; ( ( V ) ) ( Array ) = ( ( V ) ) ( 0 ) ; # loop ( ( V ) ) ( I ) = 1 ; (( ( ( V ) "
        ") ( Found_len ) ; ( ( V ) ) ( 1Temp ) = ( ( V ) ) ( Array ) ( 0 ) ; ( ( V ) ) ( 2Temp ) = "
        "( ( V ) ) ( Array ) ( ( ( V ) ) ( I ) ) ; ( ( V ) ) ( Array ) ( 0 ) = 0 ; ( ( V ) ) ( "
        "Array ) ( ( ( V ) ) ( I ) ) = 1 ; ( ( V ) ) ( Found_len ) = ( ( V ) ) ( Array ) ( 0 ) = 1 "
        "; ( ( V ) ) ( Array ) ( 0 ) = ( ( V ) ) ( 1Temp ) ; ( ( V ) ) ( Array ) ( ( ( V ) ) ( I ) "
        ") = ( ( V ) ) ( 2Temp ) ; ( ( V ) ) ( I ) = ( ( V ) ) ( I ) + 1 ; )) ; ( ( V ) ) ( I ) = "
        "( ( V ) ) ( I ) - 1 ; ( R ) = ( ( V ) ) ( I ) ; ( ( V ) ) = 0 ; ( V ) = ( V ) - 1 * 1 ; "
        ")) ; # ((str)) ( F_print_str ) = (( ( ( V ) + 1 * 1 ) ; ( V ) = ( V ) + 1 * 1 ; ( ( V ) ) "
        "( Str ) = ( ( V ) ) ( 0 ) ; ( ( V ) + 1 * 1 ) = (( ( ( V ) ) ( Str ) )) ; ( F_len ) ; ( ( "
        "V ) ) ( Len ) = ( R ) ; # loop ( ( ( V ) ) ( I ) ) = 0 ; (( ( ( V ) ) ( I ) < ( ( V ) ) ( "
        "Len ) ; ( _charstd_output ) = ( ( V ) ) ( Str ) ( ( ( V ) ) ( I ) ) ; ( _outputstd ) ; ( "
        "( V ) ) ( I ) = ( ( V ) ) ( I ) + 1 ; )) ; ( _charstd_output ) = 01 ; ( _outputstd ) ; ( "
        "( V ) ) = 0 ; ( V ) = ( V ) - 1 * 1 ; )) ; # Hello world\\n ( ( V ) + 1 * 1 ) = (( (( 27 "
        "; 011 ; 081 ; 081 ; 111 ; 23 ; 78 ; 111 ; 114 ; 081 ; 001 ; 33 ; 01 )) )) ; ( F_print_str "
        ") ; ( S ) = 0 ; ");
    // EXPECT_EQ((diagnostic::Range{{{0}, {0}}, {{0}, {0}}}),
    //           (diagnostic::Range{{{0}, {0}}, {{0}), {0}}}));
}
