#include "lib/number.cpp"
#include <gtest/gtest.h>

TEST(Number, LexicographicallyMinimalRotation) {
    EXPECT_EQ(number::lexicographically_minimal_rotation("dbca"), "adbc");
    EXPECT_EQ(number::lexicographically_minimal_rotation("abbab"), "ababb");
    EXPECT_EQ(number::lexicographically_minimal_rotation("aabaaa"), "aaaaab");
    EXPECT_EQ(number::lexicographically_minimal_rotation(""), "");
}
