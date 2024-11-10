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

TEST(Number, PlusMinusMultiplyDivide) {
    auto pi = number::Value(1);
    EXPECT_EQ(pi.to_string(), "{0 1}{1}");

    auto num = number::Value(1) * pi * pi;
    EXPECT_EQ(num.to_string(), "{0 0 0 1}{1}");

    num = number::Value(24) / pi;
    EXPECT_EQ(num.to_string(), "{24}{1}");

    num = number::Value(1) * pi * pi + number::Value(3) * pi - number::Value(10) -
          number::Value(24) / pi;
    EXPECT_EQ(num.to_string(), "{-24 -10 3 1}{1}");

    auto den = number::Value(1) * pi * pi + number::Value(2) * pi - number::Value(1) -
               number::Value(2) / pi;
    EXPECT_EQ(den.to_string(), "{-2 -1 2 1}{1}");

    num = num * number::Value(100);
    EXPECT_EQ(num.to_string(), "{0 -2400 -1000 300 100}{1}");
    den = den * number::Value(10) * pi;
    EXPECT_EQ(den.to_string(), "{0 0 -20 -10 20 10}{1}");

    EXPECT_EQ((num / den).to_string(), "{-240 -100 30 10}{0 -2 -1 2 1}");

    EXPECT_EQ((pi * pi + pi - pi * pi).to_string(), "{0 1}{1}");
}

TEST(Number, ValueFromName) {
    auto n = number::Value("abcd");
    EXPECT_EQ(n.get_numerator(), (std::vector<BigInt>{pow(number::LETTER_BASE, 0) * 'a',
                                                      pow(number::LETTER_BASE, 1) * 'b',
                                                      pow(number::LETTER_BASE, 2) * 'c',
                                                      pow(number::LETTER_BASE, 3) * 'd'}));
    n = number::Value("bcda");
    EXPECT_EQ(n.get_numerator(), (std::vector<BigInt>{pow(number::LETTER_BASE, 0) * 'a',
                                                      pow(number::LETTER_BASE, 1) * 'b',
                                                      pow(number::LETTER_BASE, 2) * 'c',
                                                      pow(number::LETTER_BASE, 3) * 'd'}));

    EXPECT_EQ(n.to_letters(), "abcd");
}
