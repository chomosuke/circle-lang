#include "lib/parser.cpp"
#include "test/sample_programs.hpp"
#include "token_to_string.cpp"
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>

void print_indent(std::ostream& ss, int indent) {
    for (int i{0}; i < indent; i++) {
        ss << "    ";
    }
}

namespace de_double_bracket {
    void print(std::ostream& ss, const Node& n, int indent) {
        ss << '\n';
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
            ss << '\n';
        }
    }
} // namespace de_double_bracket

TEST(Parse, DeDoubleBracket_HelloWorld) {
    std::stringstream ss;
    auto diags = std::vector<diag::Diagnostic>();
    auto tokens = *lex(sample_programs::HELLO_WORLD, diags);
    EXPECT_TRUE(diags.empty());
    auto debracketed = *de_double_bracket::parse(tokens, diags);
    EXPECT_TRUE(diags.empty());
    de_double_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), sample_programs::HELLO_WORLD_EXPECTED);

    auto range_2nd_inner_array =
        std::get<de_double_bracket::Node>(debracketed.elements[3][4].t).elements[7][0].range;
    EXPECT_EQ(range_2nd_inner_array.to_string(), "47:2-54:3");
}

TEST(Parse, DeDoubleBracket_MissingClosingBrackets) {
    auto diags = std::vector<diag::Diagnostic>();
    const char* const missing_2_close_b = "((\n"
                                          "( (V) + 1*1 );\n"
                                          "(V) := (V) + 1*1;\n"
                                          "( (V) )(Array) := (( ( (V) )(0);\n"
                                          "\n";
    auto tokens = *lex(missing_2_close_b, diags);
    EXPECT_TRUE(diags.empty());

    EXPECT_FALSE(de_double_bracket::parse(tokens, diags));
    EXPECT_EQ(to_string(std::move(diags)), "[ERROR] 1:1-1:2: Can not find matching \"))\"\n"
                                           "[ERROR] 4:19-4:20: Can not find matching \"))\"\n");
}

TEST(Parse, DeDoubleBracket_MissingOpenBrackets) {
    auto diags = std::vector<diag::Diagnostic>();
    const char* const missing_2_open_b = "( (V) + 1*1 );\n"
                                         "(V) := (V) + 1*1 ));\n"
                                         "( (V) )(Array) := ( (V) )(0) ));\n";

    auto tokens = *lex(missing_2_open_b, diags);
    EXPECT_TRUE(diags.empty());

    EXPECT_FALSE(de_double_bracket::parse(tokens, diags));
    EXPECT_EQ(to_string(std::move(diags)), "[ERROR] 2:18-2:19: Can not find matching \"((\"\n"
                                           "[ERROR] 3:30-3:31: Can not find matching \"((\"\n");
}

TEST(Parse, DeDoubleBracket_ExtraSemi) {
    std::stringstream ss;
    auto diags = std::vector<diag::Diagnostic>();
    const char* const extra_semi = "((\n"
                                   ";( (V) + 1*1 );\n"
                                   "(V) := (V) + 1*1;;\n"
                                   "( (V) )(Array) := (( ( (V) )(0) ));\n"
                                   "));\n";
    auto tokens = *lex(extra_semi, diags);
    EXPECT_TRUE(diags.empty());

    auto debracketed = *de_double_bracket::parse(tokens, diags);
    de_double_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), R"(
((
    ( ( V ) + {0 1}{1} * {0 1}{1} )
    ( V ) := ( V ) + {0 1}{1} * {0 1}{1}
    ( ( V ) )( Array ) := ((
        ( ( V ) )( {0 0}{1} )
    ))
))
)");
    EXPECT_EQ(to_string(std::move(diags)), "[WARNING] 2:1-2:1: Extra ';' found\n"
                                           "[WARNING] 3:18-3:18: Extra ';' found\n");
}

namespace de_bracket {
    void print(std::ostream& ss, const DoubleBracket& n, int indent);

    void print(std::ostream& ss, const SingleBracket& n, int indent) {
        ss << "( ";
        for (const auto& child : n.children) {
            std::visit(
                [&]<typename T>(const T& t) {
                    if constexpr (std::is_same_v<T, DoubleBracket>) {
                        ss << "((";
                        print(ss, t, indent + 1);
                        print_indent(ss, indent);
                        ss << "))";
                    } else if constexpr (std::is_same_v<T, SingleBracket>) {
                        print(ss, t, indent);
                    } else if constexpr (std::is_same_v<T, token::Number> ||
                                         std::is_same_v<T, token::Assign> ||
                                         std::is_same_v<T, token::OperatorBinary> ||
                                         std::is_same_v<T, token::OperatorUnary>) {
                        ss << token::to_string(t);
                    } else {
                        static_assert(false, "Non exhaustive");
                    }
                },
                child.t);
        }
        ss << " )";
    }

    void print(std::ostream& ss, const DoubleBracket& n, int indent) {
        ss << '\n';
        for (const auto& element : n.elements) {
            print_indent(ss, indent);
            for (const auto& token : element) {
                std::visit(
                    [&]<typename T>(const T& t) {
                        if constexpr (std::is_same_v<T, DoubleBracket>) {
                            ss << "((";
                            print(ss, t, indent + 1);
                            print_indent(ss, indent);
                            ss << "))";
                        } else if constexpr (std::is_same_v<T, SingleBracket>) {
                            print(ss, t, indent);
                        } else if constexpr (std::is_same_v<T, token::Number> ||
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
            ss << '\n';
        }
    }
} // namespace de_bracket

TEST(Parse, DeSingleBracket_HelloWorld) {
    std::stringstream ss;
    auto diags = std::vector<diag::Diagnostic>();
    auto tokens = *lex(sample_programs::HELLO_WORLD, diags);
    EXPECT_TRUE(diags.empty());
    auto de_double_bracketed = *de_double_bracket::parse(tokens, diags);
    EXPECT_TRUE(diags.empty());
    auto debracketed = de_bracket::parse(std::move(de_double_bracketed), diags);
    de_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), sample_programs::HELLO_WORLD_EXPECTED);

    auto range_1st_charstd_output =
        std::get<de_bracket::DoubleBracket>(
            std::get<de_bracket::DoubleBracket>(debracketed.elements[3][2].t).elements[7][0].t)
            .elements[1][0]
            .range;
    EXPECT_EQ(range_1st_charstd_output.to_string(), "50:3-50:19");
}

TEST(Parse, DeBracket_MissingClosingBrackets) {
    std::stringstream ss;
    auto diags = std::vector<diag::Diagnostic>();
    const char* const missing_close_b = "((\n"
                                          "( (V) + 1*1 );\n"
                                          "(V) := (V + 1*1;\n"
                                          "( (V) )(Array) := (( ( (V (0) ));\n"
                                          "));\n";
    auto tokens = *lex(missing_close_b, diags);
    EXPECT_TRUE(diags.empty());

    auto de_double_bracketed = *de_double_bracket::parse(tokens, diags);
    EXPECT_TRUE(diags.empty());

    auto debracketed = de_bracket::parse(std::move(de_double_bracketed), diags);
    EXPECT_EQ(to_string(std::move(diags)), "[ERROR] 3:8-3:8: Can not find matching ')'\n"
                                           "[ERROR] 4:22-4:22: Can not find matching ')'\n"
                                           "[ERROR] 4:24-4:24: Can not find matching ')'\n"
                                           );

    de_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), R"(
((
    ( ( V ) + {0 1}{1} * {0 1}{1} )
    ( V ) := V + {0 1}{1} * {0 1}{1}
    ( ( V ) )( Array ) := ((
        V( {0 0}{1} )
    ))
))
)");
}

TEST(Parse, DeBracket_MissingOpenBrackets) {
    std::stringstream ss;
    auto diags = std::vector<diag::Diagnostic>();
    const char* const missing_open_b = "((\n"
                                         "( (V) + 1*1 );\n"
                                         "(V) := V) + 1*1;\n"
                                         "( (V) )(Array) := (( V) )(0) ));\n"
                                         "));\n";

    auto tokens = *lex(missing_open_b, diags);
    EXPECT_TRUE(diags.empty());

    auto de_double_bracketed = *de_double_bracket::parse(tokens, diags);
    EXPECT_TRUE(diags.empty());

    auto debracketed = de_bracket::parse(std::move(de_double_bracketed), diags);
    EXPECT_EQ(to_string(std::move(diags)), "[ERROR] 3:9-3:9: Can not find matching '('\n"
                                           "[ERROR] 4:23-4:23: Can not find matching '('\n"
                                           "[ERROR] 4:25-4:25: Can not find matching '('\n");

    de_bracket::print(ss, debracketed, 0);
    EXPECT_EQ(ss.str(), R"(
((
    ( ( V ) + {0 1}{1} * {0 1}{1} )
    ( V ) := V + {0 1}{1} * {0 1}{1}
    ( ( V ) )( Array ) := ((
        V( {0 0}{1} )
    ))
))
)");
}
