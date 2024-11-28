#include "lib/from_brainfuck.cpp"
#include "lib/config.hpp"
#include "lib/interpret.hpp"
#include <gtest/gtest.h>

TEST(FromBrainfuck, Sum) {
    auto src_code = from_brainfuck(",>,[<+>-]<.");
    std::stringstream out{};
    std::stringstream in{"!#"};
    std::stringstream err{};
    interpret(src_code, in, out, err, Config{.debug{false}});
    EXPECT_EQ(err.str(), "");
    EXPECT_EQ(out.str(), "D");
}

TEST(FromBrainfuck, HelloWorld) {
    auto src_code = from_brainfuck("+++++++++++[>++++++>+++++++++>++++++++>++++>+++>+<<<<<<-]>+++"
                                   "+++.>++.+++++++..+++.>>.>-.<<-.<.+++.------.--------.>>>+.>-.");
    std::stringstream out{};
    std::stringstream in{};
    std::stringstream err{};
    interpret(src_code, in, out, err, Config{.debug{false}});
    EXPECT_EQ(err.str(), "");
    EXPECT_EQ(out.str(), "Hello, World!\n");
}
