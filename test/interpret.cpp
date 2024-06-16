#include "../lib/interpret.hpp"
#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(Interpret, HelloWorld) {
    std::stringstream ss{};
    std::stringstream s_bin{};
    interpret("", s_bin, ss, s_bin);
    EXPECT_EQ(ss.str(), "Hello interpreter\n");
}
