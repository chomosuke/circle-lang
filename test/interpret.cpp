#include "lib/interpret.cpp"
#include "test/sample_programs.hpp"
#include <gtest/gtest.h>

TEST(Interpret, HelloWorld) {
    std::stringstream out{};
    std::stringstream in{};
    std::stringstream err{};
    interpret(sample_programs::HELLO_WORLD, in, out, err, Config{.debug{false}});
    EXPECT_EQ(err.str(), "");
    EXPECT_EQ(out.str(), "Hello world!\n");
}
