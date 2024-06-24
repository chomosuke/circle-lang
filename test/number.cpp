#include "lib/number.cpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <random>

int bf_lexicographically_minimal_rotation(std::string_view str) {
    number::RotateableIndex<std::string_view, char> strr{str};
    number::RotateableIndex<std::string_view, char> min_strr{str};
    for (int i{0}; i < str.size(); i++) {
        strr.set_offset(i);
        for (int j{0}; j < strr.size(); j++) {
            if (strr[j] < min_strr[j]) {
                min_strr.set_offset(i);
                break;
            }
            if (strr[j] > min_strr[j]) {
                break;
            }
        }
    }
    return min_strr.get_offset();
}

TEST(Number, LexicographicallyMinimalRotation) {
    EXPECT_EQ(number::lexicographically_minimal_rotation("dbca").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("abbab").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("aabaaa").get_offset(), 3);
    EXPECT_EQ(number::lexicographically_minimal_rotation("").get_offset(), 0);
    EXPECT_EQ(number::lexicographically_minimal_rotation("ababbababababbababb").get_offset(), 5);
    EXPECT_EQ(number::lexicographically_minimal_rotation("1111111101").get_offset(), 8);
}

TEST(Number, LexicographicallyMinimalRotationBF) {
    std::mt19937 rng{0};
    std::uniform_int_distribution<std::mt19937::result_type> bool_dist(0, 9);
    for (int i{0}; i < 1000; i++) {
        std::stringstream ss;
        for (int j{0}; j < 10 + i / 10; j++) {
            ss << bool_dist(rng) % (i % 9 + 2);
        }
        ASSERT_EQ(number::lexicographically_minimal_rotation(ss.str()).get_offset(),
                  bf_lexicographically_minimal_rotation(ss.str()))
            << ss.str() << " " << i;
        // std::cout << ss.str() << " " << i << std::endl;
    }
}
