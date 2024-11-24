#include "lib/lexer.cpp"
#include "test/sample_programs.hpp"
#include "token_to_string.cpp"
#include <gtest/gtest.h>
#include <variant>

std::string tokens_to_string(const std::vector<token::Token>& ts) {
    std::stringstream ss{};
    for (const auto& t : ts) {
        ss << std::visit([](auto&& t) { return token::to_string(t); }, t.t);
    }
    return ss.str();
}

TEST(Lex, Empty) {
    auto diags = diag::Diags();
    EXPECT_EQ(tokens_to_string(lex(R"()", diags).value()), "");
    EXPECT_TRUE(diags.empty());
}

TEST(Lex, InvalidOperator) {
    auto diags = diag::Diags();
    auto invalid_op = lex("(( (V) + 1*1 );\n"
                          "(V) := (V) + 1**1;\n"
                          "( (V) )(Array) := ( (V) )(0);)\n",
                          diags);
    EXPECT_EQ(diags.to_string(), "[ERROR] 2:15-2:16: \"**\" is not a valid operator.\n");
}

TEST(Lex, HelloWorld) {
    auto diags = diag::Diags();
    auto hello_world = *lex(sample_programs::HELLO_WORLD, diags);
    EXPECT_TRUE(diags.empty());

    EXPECT_EQ(tokens_to_string(hello_world), sample_programs::HELLO_WORLD_TOKEN);
    EXPECT_EQ(hello_world[hello_world.size() - 9].range.to_string(), "56:2-56:12");
}

TEST(Lex, AllOperators) {
    auto diags = diag::Diags();
    EXPECT_EQ(tokens_to_string(lex(R"(+ - * / && || = != < > <= >= !)", diags).value()),
              " +  -  *  /  &&  ||  =  !=  <  >  <=  >= !");
    EXPECT_TRUE(diags.empty());
}
