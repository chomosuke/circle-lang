#include "lib/number.cpp"
#include <gtest/gtest.h>

TEST(Number, LexicographicallyMinimalRotation) {
    EXPECT_EQ(number::lexicographically_minimal_rotation("dbca").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("abbab").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("aabaaa").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("").get_offset(), 0);
    EXPECT_EQ(number::lexicographically_minimal_rotation("ababbababababbababb").get_offset(), 5);
}
