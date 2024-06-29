#include "lib/lexer.cpp"
#include "test/sample-programs.hpp"
#include <gtest/gtest.h>
#include <variant>

namespace token {
    std::string token_to_string(Token t) {
        return std::visit(
            [&]<typename T>(T& t) -> std::string {
                if constexpr (std::is_same_v<T, OpenBracket>) {
                    return "(";
                } else if constexpr (std::is_same_v<T, CloseBracket>) {
                    return ")";
                } else if constexpr (std::is_same_v<T, Semicolon>) {
                    return ";";
                } else if constexpr (std::is_same_v<T, OpenBracket2>) {
                    return "((";
                } else if constexpr (std::is_same_v<T, CloseBracket2>) {
                    return "))";
                } else if constexpr (std::is_same_v<T, Comment>) {
                    return std::format("#{}", t.content);
                } else if constexpr (std::is_same_v<T, Number>) {
                    auto letters = t.value.to_letters();
                    if (letters) {
                        return *letters;
                    }

                    std::stringstream ss{};
                    ss << "{";
                    const auto* space = "";
                    for (const auto& n : t.value.get_numerator()) {
                        ss << space << n;
                        space = " ";
                    }
                    ss << "}{";
                    space = "";
                    for (const auto& n : t.value.get_denominator()) {
                        ss << space << n;
                        space = " ";
                    }
                    ss << "}";
                    return ss.str();
                } else if constexpr (std::is_same_v<T, Assign>) {
                    return ":=";
                } else if constexpr (std::is_same_v<T, OperatorBinary>) {
                    switch (t.kind) {
                    case number::op::plus:
                        return "+";
                    case number::op::minus:
                        return "-";
                    case number::op::multiply:
                        return "*";
                    case number::op::divide:
                        return "/";
                    case number::op::remainder:
                        return "%";
                    case number::op::bool_and:
                        return "&&";
                    case number::op::bool_or:
                        return "||";
                    case number::op::equal:
                        return "=";
                    case number::op::not_equal:
                        return "!=";
                    case number::op::smaller:
                        return "<";
                    case number::op::smaller_or_equal:
                        return "<=";
                    case number::op::greater:
                        return ">";
                    case number::op::greater_or_equal:
                        return ">=";
                    }
                } else if constexpr (std::is_same_v<T, OperatorUnary>) {
                    switch (t.kind) {
                    case number::op::bool_not:
                        return "!";
                    }
                } else {
                    static_assert(false, "Not exhaustive");
                }
            },
            t.kind);
    }

    std::string tokens_to_string(const std::vector<Token>& ts) {
        std::stringstream ss{};
        for (const auto& t : ts) {
            ss << token_to_string(t) << " ";
        }
        return ss.str();
    }
} // namespace token

TEST(Lex, HelloWorld) {
    EXPECT_EQ(tokens_to_string(lex(R"()").value()), "");
    EXPECT_EQ(lex("(( (V) + 1*1 );\n"
                  "(V) := (V) + 1**1;\n"
                  "( (V) )(Array) := ( (V) )(0);)\n")
                  .error()
                  .to_string(),
              R"(2:15-2:16: "**" is not a valid operator.)");

    auto hello_world = lex(sample_programs::HELLO_WORLD).value();
    EXPECT_EQ(
        tokens_to_string(hello_world),
        "( S ) ; ( V ) := V_a ; # ((array)) -> len ( F_len ) := (( ( ( V ) + {0 1}{1} * {0 1}{1} ) "
        "; ( V ) := ( V ) + {0 1}{1} * {0 1}{1} ; ( ( V ) ) ( Array ) := ( ( V ) ) ( {0 0}{1} ) ; "
        "# loop ( ( V ) ) ( I ) := {0 1}{1} ; (( ( ( V ) ) ( Found_len ) ; ( ( V ) ) ( 1Temp ) := "
        "( ( V ) ) ( Array ) ( {0 0}{1} ) ; ( ( V ) ) ( 2Temp ) := ( ( V ) ) ( Array ) ( ( ( V ) ) "
        "( I ) ) ; ( ( V ) ) ( Array ) ( {0 0}{1} ) := {0 0}{1} ; ( ( V ) ) ( Array ) ( ( ( V ) ) "
        "( I ) ) := {0 1}{1} ; ( ( V ) ) ( Found_len ) := ( ( V ) ) ( Array ) ( {0 0}{1} ) = {0 "
        "1}{1} ; ( ( V ) ) ( Array ) ( {0 0}{1} ) := ( ( V ) ) ( 1Temp ) ; ( ( V ) ) ( Array ) ( ( "
        "( V ) ) ( I ) ) := ( ( V ) ) ( 2Temp ) ; ( ( V ) ) ( I ) := ( ( V ) ) ( I ) + {0 1}{1} ; "
        ")) ; ( ( V ) ) ( I ) := ( ( V ) ) ( I ) - {0 1}{1} ; ( R ) := ( ( V ) ) ( I ) ; ( ( V ) ) "
        ":= {0 0}{1} ; ( V ) := ( V ) - {0 1}{1} * {0 1}{1} ; )) ; # ((str)) ( F_print_str ) := (( "
        "( ( V ) + {0 1}{1} * {0 1}{1} ) ; ( V ) := ( V ) + {0 1}{1} * {0 1}{1} ; ( ( V ) ) ( Str "
        ") := ( ( V ) ) ( {0 0}{1} ) ; ( ( V ) + {0 1}{1} * {0 1}{1} ) := (( ( ( V ) ) ( Str ) )) "
        "; ( F_len ) ; ( ( V ) ) ( Len ) := ( R ) ; # loop ( ( ( V ) ) ( I ) ) := {0 0}{1} ; (( ( "
        "( V ) ) ( I ) < ( ( V ) ) ( Len ) ; ( _charstd_output ) := ( ( V ) ) ( Str ) ( ( ( V ) ) "
        "( I ) ) ; ( _outputstd ) ; ( ( V ) ) ( I ) := ( ( V ) ) ( I ) + {0 1}{1} ; )) ; ( "
        "_charstd_output ) := {0 10}{1} ; ( _outputstd ) ; ( ( V ) ) := {0 0}{1} ; ( V ) := ( V ) "
        "- {0 1}{1} * {0 1}{1} ; )) ; # Hello world\\n ( ( V ) + {0 1}{1} * {0 1}{1} ) := (( (( {0 "
        "72}{1} ; {0 101}{1} ; {0 108}{1} ; {0 108}{1} ; {0 111}{1} ; {0 32}{1} ; {0 87}{1} ; {0 "
        "111}{1} ; {0 114}{1} ; {0 108}{1} ; {0 100}{1} ; {0 33}{1} ; {0 10}{1} )) )) ; ( "
        "F_print_str ) ; ( S ) := {0 0}{1} ; ");
    EXPECT_EQ(hello_world[hello_world.size() - 9].range,
              (diagnostic::Range{.start{{65}, {1}}, .end{{65}, {12}}}));
}
