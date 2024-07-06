#include "lib/parser.cpp"
#include "test/sample_programs.hpp"
#include "token_to_string.cpp"
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

namespace de_double_bracket {
    void print_indent(std::ostream& ss, int indent) {
        for (int i{0}; i < indent; i++) {
            ss << "    ";
        }
    }

    void print(std::ostream& ss, const Node& n, int indent) {
        ss << std::endl;
        for (const auto& element : n.elements) {
            print_indent(ss, indent);
            for (const auto& token : element) {
                std::visit(
                    [&]<typename T>(const T& t) {
                        if constexpr (std::is_same_v<T, Node>) {
                            ss << "((";
                            print(ss, t, indent + 1);
                            print_indent(ss, indent);
                            ss << "))";
                        } else if constexpr (std::is_same_v<T, token::OpenBracket> ||
                                             std::is_same_v<T, token::CloseBracket> ||
                                             std::is_same_v<T, token::Number> ||
                                             std::is_same_v<T, token::Assign> ||
                                             std::is_same_v<T, token::OperatorBinary> ||
                                             std::is_same_v<T, token::OperatorUnary>) {
                            ss << token::to_string(t);
                        } else {
                            static_assert(false, "Non exhaustive");
                        }
                    },
                    token.t);
            }
            ss << std::endl;
        }
    }
} // namespace de_double_bracket

TEST(Parse, DeDoubleBracket) {
    std::stringstream ss;
    auto tokens = lex(sample_programs::HELLO_WORLD).value();
    std::vector<diagnostic::Range> ranges;
    auto debracketed = de_double_bracket::parse(tokens, ranges).value().first;
    de_double_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), R"(
( S )
( V ) := V_a
( F_len ) := ((
    ( ( V ) + {0 1}{1} * {0 1}{1} )
    ( V ) := ( V ) + {0 1}{1} * {0 1}{1}
    ( ( V ) )( Array ) := ( ( V ) )( {0 0}{1} )
    ( ( V ) )( I ) := {0 1}{1}
    ((
        ( ( V ) )( Found_len )
        ( ( V ) )( 1Temp ) := ( ( V ) )( Array )( {0 0}{1} )
        ( ( V ) )( 2Temp ) := ( ( V ) )( Array )( ( ( V ) )( I ) )
        ( ( V ) )( Array )( {0 0}{1} ) := {0 0}{1}
        ( ( V ) )( Array )( ( ( V ) )( I ) ) := {0 1}{1}
        ( ( V ) )( Found_len ) := ( ( V ) )( Array )( {0 0}{1} ) = {0 1}{1}
        ( ( V ) )( Array )( {0 0}{1} ) := ( ( V ) )( 1Temp )
        ( ( V ) )( Array )( ( ( V ) )( I ) ) := ( ( V ) )( 2Temp )
        ( ( V ) )( I ) := ( ( V ) )( I ) + {0 1}{1}
    ))
    ( ( V ) )( I ) := ( ( V ) )( I ) - {0 1}{1}
    ( R ) := ( ( V ) )( I )
    ( ( V ) ) := {0 0}{1}
    ( V ) := ( V ) - {0 1}{1} * {0 1}{1}
))
( F_print_str ) := ((
    ( ( V ) + {0 1}{1} * {0 1}{1} )
    ( V ) := ( V ) + {0 1}{1} * {0 1}{1}
    ( ( V ) )( Str ) := ( ( V ) )( {0 0}{1} )
    ( ( V ) + {0 1}{1} * {0 1}{1} ) := ((
        ( ( V ) )( Str )
    ))
    ( F_len )
    ( ( V ) )( Len ) := ( R )
    ( ( ( V ) )( I ) ) := {0 0}{1}
    ((
        ( ( V ) )( I ) < ( ( V ) )( Len )
        ( _charstd_output ) := ( ( V ) )( Str )( ( ( V ) )( I ) )
        ( _outputstd )
        ( ( V ) )( I ) := ( ( V ) )( I ) + {0 1}{1}
    ))
    ( _charstd_output ) := {0 10}{1}
    ( _outputstd )
    ( ( V ) ) := {0 0}{1}
    ( V ) := ( V ) - {0 1}{1} * {0 1}{1}
))
( ( V ) + {0 1}{1} * {0 1}{1} ) := ((
    ((
        {0 72}{1}
        {0 101}{1}
        {0 108}{1}
        {0 108}{1}
        {0 111}{1}
        {0 32}{1}
        {0 87}{1}
        {0 111}{1}
        {0 114}{1}
        {0 108}{1}
        {0 100}{1}
        {0 33}{1}
        {0 10}{1}
    ))
))
( F_print_str )
( S ) := {0 0}{1}
)");
    auto range_2nd_inner_array =
        std::get<de_double_bracket::Node>(debracketed.elements[3][4].t).elements[7][0].range;
    EXPECT_EQ(range_2nd_inner_array.to_string(), "47:2-54:3");

    const char* const missing_2_close_b = "((\n"
                                          "( (V) + 1*1 );\n"
                                          "(V) := (V) + 1*1;\n"
                                          "( (V) )(Array) := (( ( (V) )(0);\n"
                                          "\n";
    tokens = lex(missing_2_close_b).value();
    ss.clear();
    ranges.clear();
    EXPECT_EQ(to_string(std::move(de_double_bracket::parse(tokens, ranges).error())),
              "1:1-1:2: Can not find matching \"))\"\n"
              "4:19-4:20: Can not find matching \"))\"\n");
    const char* const missing_2_open_b = "( (V) + 1*1 );\n"
                                         "(V) := (V) + 1*1 ));\n"
                                         "( (V) )(Array) := ( (V) )(0) ));\n";
    tokens = lex(missing_2_open_b).value();
    ss.clear();
    ranges.clear();
    EXPECT_EQ(to_string(std::move(de_double_bracket::parse(tokens, ranges).error())),
              "2:18-2:19: Can not find matching \"((\"\n"
              "3:30-3:31: Can not find matching \"((\"\n");
}
