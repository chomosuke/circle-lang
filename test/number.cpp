#include "lib/number.cpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <numbers>
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

TEST(Number, PlusMinusMultiplyDivide) {
    auto pi = number::Value(1);
    EXPECT_EQ(pi.to_string(), "{0 1}{1}");

    auto num = number::Value(1) * pi * pi;
    EXPECT_EQ(num.to_string(), "{0 0 0 1}{1}");

    num = number::Value(24) / pi;
    EXPECT_EQ(num.to_string(), "{24}{1}");

    num = (number::Value(1) + number::Value(2) / pi) * (number::Value(1) - number::Value(3) / pi) *
          (number::Value(1) + number::Value(4) / pi);
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

TEST(Number, Bool) {
    auto pi = number::Value(1);
    // NOLINTNEXTLINE(misc-redundant-expression)
    auto one = pi / pi;
    auto num = one.clone();
    for (auto i = 1; i <= 12; i++) {
        num = num * (pi - number::Value(i) / pi);
    }

    auto f = one.clone();
    for (auto i = 13; i <= 16; i++) {
        f = f * (pi - number::Value(i) / pi);
    }

    auto num2 = num * f / f;
    EXPECT_TRUE((num == num2).to_bool());
    EXPECT_FALSE((num != num2).to_bool());
    EXPECT_FALSE((num > num2).to_bool());
    EXPECT_TRUE((num >= num2).to_bool());
    EXPECT_FALSE((num < num2).to_bool());
    EXPECT_TRUE((num <= num2).to_bool());

    auto num3 = num2 + one;
    EXPECT_TRUE((num3 > num).to_bool());
    EXPECT_FALSE((num > num3).to_bool());
    EXPECT_TRUE((num3 >= num).to_bool());
    EXPECT_FALSE((num >= num3).to_bool());
    EXPECT_TRUE((num < num3).to_bool());
    EXPECT_FALSE((num3 < num).to_bool());
    EXPECT_TRUE((num <= num3).to_bool());
    EXPECT_FALSE((num3 <= num).to_bool());
    EXPECT_TRUE((num3 != num).to_bool());
    EXPECT_FALSE((num3 == num).to_bool());

    auto bignum = 10000000000;
    auto num4 = number::Value(bignum) * pi;
    auto num5 = number::Value(static_cast<long>(static_cast<double>(bignum) * std::numbers::pi));
    EXPECT_TRUE((num4 > num5).to_bool());
}

TEST(Number, Index) {
    auto pi = number::Value(1);
    // NOLINTNEXTLINE(misc-redundant-expression)
    auto one = pi / pi;
    auto num = one.clone();
    for (auto i = 1; i <= 12; i++) {
        num = num * (pi - number::Value(i) / pi);
    }

    auto f = one.clone();
    for (auto i = 13; i <= 16; i++) {
        f = f * (pi - number::Value(i) / pi);
    }

    auto num2 = num * f / f;
    auto length = 256;

    auto map = std::unordered_map<number::Index, bool>();

    auto ind1 = number::Index(num.clone(), length);
    auto ind2 = number::Index::make_ref(num2, length);
    auto ind3 = number::Index(num2 + number::Value(length), length);
    auto ind4 = number::Index(num + one, length);
    auto ind5 = number::Index(num + pi, length);
    EXPECT_EQ(ind1.hash(), ind2.hash());
    EXPECT_EQ(ind1.hash(), ind3.hash());
    EXPECT_NE(ind1.hash(), ind4.hash());
    EXPECT_NE(ind1.hash(), ind5.hash());

    map.insert(std::make_pair(ind1.clone(), true));

    EXPECT_TRUE(map.contains(ind1));
    EXPECT_TRUE(map.contains(ind2));
    EXPECT_TRUE(map.contains(ind3));
    EXPECT_FALSE(map.contains(ind4));
    EXPECT_FALSE(map.contains(ind5));
}
