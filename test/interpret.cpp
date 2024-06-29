#include "lib/interpret.cpp"
#include "test/sample_programs.hpp"
#include <gtest/gtest.h>

TEST(Interpret, HelloWorld) {
    std::stringstream ss{};
    std::stringstream s_nil{};
    // interpret(sample_programs::HELLO_WORLD, s_nil, ss, s_nil);
    // EXPECT_EQ(ss.str(), "Hello world\n");
}
