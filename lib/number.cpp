#include "number.hpp"

#include "macros.hpp"
#include "pi.hpp"

#include "vendor/BigInt.hpp"
#include <cassert>
#include <climits>
#include <cmath>
#include <iterator>
#include <sstream>
#include <string_view>
#include <tl/expected.hpp>
#include <vector>

namespace number {
    bool is_in_char_set(char c) {
        return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') ||
               c == '_';
    }

    template <typename T, typename I> class RotateableIndex {
      private:
        const T m_t;
        int m_offset{};

      public:
        explicit RotateableIndex(const T& t) : m_t{t} {}

        const I& operator[](std::size_t i) const { return m_t[(i + m_offset) % m_t.size()]; }

        void rotate(int n) { m_offset += n; }

        void set_offset(int offset) { m_offset = offset; }
        [[nodiscard]] int get_offset() const { return m_offset; }

        [[nodiscard]] T::size_type size() const { return m_t.size(); }

        bool operator==(const RotateableIndex& rhs) const = default;

        class Iterator {
          private:
            std::reference_wrapper<const RotateableIndex> m_r;
            int m_index{};

          public:
            explicit Iterator(const RotateableIndex& v, int index) : m_r{v}, m_index{index} {}

            using value_type = I;        // NOLINT(readability-identifier-naming)
            using difference_type = int; // NOLINT(readability-identifier-naming)

            const I& operator*() const { return m_r.get()[m_index % m_r.get().size()]; }

            Iterator& operator++() {
                m_index++;
                return *this;
            }

            Iterator operator++(int) {
                auto tmp = *this;
                m_index++;
                return tmp;
            }

            bool operator==(const Iterator& rhs) const {
                return m_r.get() == rhs.m_r.get() && m_index == rhs.m_index;
            };
        };
        static_assert(std::input_iterator<Iterator>);

        Iterator begin() { return Iterator(*this, 0); }
        Iterator end() { return Iterator(*this, m_t.size()); }
    };

    RotateableIndex<std::string_view, char>
    lexicographically_minimal_rotation(std::string_view str) {
        RotateableIndex<std::string_view, char> strr{str};
        if (str.size() == 0) {
            return strr;
        }

        std::vector<int> f(str.size() * 2, 0);
        for (int i{1}; i < f.size(); i++) {
            int prev_len = f[i - 1];
            while (true) {
                if (strr[i] == strr[prev_len]) {
                    f[i] = prev_len + 1;
                    break;
                } else if (strr[i] < strr[prev_len]) {
                    strr.rotate(i - prev_len);
                    if (prev_len > 0) {
                        i = prev_len - 1;
                    } else {
                        i = 0;
                    }
                    break;
                }
                if (prev_len == 0) {
                    f[i] = 0;
                    break;
                }
                prev_len = f[prev_len - 1];
            }
        }

        return strr;
    }

    Value::Value(std::string_view letters) : m_denominator{1} {
        auto min_letters{lexicographically_minimal_rotation(letters)};

        BigInt base{1};
        for (auto l : min_letters) {
            m_numerator.push_back(base * l);
            base *= LETTER_BASE;
        }
    }

    Value::Value(const BigInt& number) : m_numerator{0, number}, m_denominator{1} {
        if (number == 0) {
            m_numerator.clear();
        }
    }

    // There is no analytical algorithm to find factorize arbitrary degree polynomial, so this is
    // best effort
    std::pair<std::vector<BigInt>, std::vector<BigInt>> simplify(const std::vector<BigInt>& num,
                                                                 const std::vector<BigInt>& den) {
        // remove trailing zero
        auto num_size = num.size();
        while (num_size > 0 && num[num_size - 1] == 0) {
            num_size--;
        }
        auto dem_size = den.size();
        while (dem_size > 0 && den[dem_size - 1] == 0) {
            dem_size--;
        }

        // factor out leading zero
        auto leading_zero_num = 0;
        auto leading_zero_dem = 0;
        while (leading_zero_num < num_size && num[leading_zero_num] == 0) {
            leading_zero_num++;
        }
        while (leading_zero_dem < dem_size && den[leading_zero_dem] == 0) {
            leading_zero_dem++;
        }
        auto leading_zero = std::min(leading_zero_num, leading_zero_dem);

        // factor out gcd
        auto g = BigInt(1);
        if (num_size > leading_zero && dem_size > leading_zero) {
            g = 0;
            for (auto i = leading_zero; i < num_size; i++) {
                g = gcd(g, num[i]);
            }
            for (auto i = leading_zero; i < dem_size; i++) {
                g = gcd(g, den[i]);
            }
        }

        auto new_num = std::vector<BigInt>();
        new_num.reserve(num_size - leading_zero);
        for (auto i = 0; i + leading_zero < num_size; i++) {
            new_num.push_back(num[i + leading_zero] / g);
        }
        auto new_den = std::vector<BigInt>();
        new_den.reserve(dem_size - leading_zero);
        for (auto i = 0; i + leading_zero < dem_size; i++) {
            new_den.push_back(den[i + leading_zero] / g);
        }

        return std::make_pair(new_num, new_den);
    }

    Value::Value(const std::vector<BigInt>& num, const std::vector<BigInt>& den) {
        auto num_den = simplify(num, den);

        m_numerator = std::move(num_den.first);
        m_denominator = std::move(num_den.second);
    }

    Value Value::clone() const {
        auto v = Value(BigInt(0));
        v.m_numerator = m_numerator;
        v.m_denominator = m_denominator;
        return v;
    }

    const std::vector<BigInt>& Value::get_numerator() const { return m_numerator; }

    const std::vector<BigInt>& Value::get_denominator() const { return m_denominator; }

    std::optional<BigInt> get_ratio(const BigInt& x, const BigInt& y) {
        if (y == 0 || x % y != 0) {
            return std::nullopt;
        }
        return x / y;
    }

    std::optional<BigInt> Value::div_pi() const {
        const auto& num = get_numerator();
        const auto& den = get_denominator();
        if (num.empty()) {
            return 0;
        }
        if (num[0] != 0 || num.size() != den.size() + 1) {
            return std::nullopt;
        }

        auto ratio = get_ratio(num.back(), den.back());
        if (!ratio) {
            return std::nullopt;
        }
        for (int i = 0; i < den.size(); i++) {
            if (ratio != get_ratio(num[i + 1], den[i])) {
                return std::nullopt;
            }
        }
        return ratio;
    }

    std::optional<std::string> Value::to_letters() const {
        if (m_numerator.size() == 0) {
            return std::nullopt;
        }
        std::stringstream ss{};
        BigInt base{1};
        for (const auto& n : m_numerator) {
            if (n % base != 0 || n / base > CHAR_MAX || n / base <= '\0') {
                return std::nullopt;
            }
            ss << static_cast<char>((n / base).to_int());
            base *= number::LETTER_BASE;
        }
        return std::make_optional(ss.str());
    }

    std::string Value::to_string() const {
        std::stringstream ss{};
        ss << "{";
        const auto* space = "";
        for (const auto& n : get_numerator()) {
            ss << space << n;
            space = " ";
        }
        ss << "}{";
        space = "";
        for (const auto& n : get_denominator()) {
            ss << space << n;
            space = " ";
        }
        ss << "}";
        return ss.str();
    }

    bool Value::to_bool() const { return !get_numerator().empty(); }

    std::vector<BigInt> operator*(const std::vector<BigInt>& lhs, const std::vector<BigInt>& rhs) {
        if (lhs.empty() || rhs.empty()) {
            return {};
        }
        auto result = std::vector<BigInt>(lhs.size() + rhs.size() - 1, 0);
        for (auto i = 0; i < lhs.size(); i++) {
            for (auto j = 0; j < rhs.size(); j++) {
                result[i + j] += lhs[i] * rhs[j];
            }
        }
        return result;
    }

    std::vector<BigInt> operator-(const std::vector<BigInt>& lhs, const std::vector<BigInt>& rhs) {
        auto result = std::vector<BigInt>(std::max(lhs.size(), rhs.size()), 0);
        for (auto i = 0; i < result.size(); i++) {
            if (i < lhs.size()) {
                result[i] += lhs[i];
            }
            if (i < rhs.size()) {
                result[i] -= rhs[i];
            }
        }
        return result;
    }

    std::vector<BigInt> plus(const std::vector<BigInt>& x, const std::vector<BigInt>& y,
                             bool positive) {
        auto size = std::max(x.size(), y.size());
        auto result = std::vector<BigInt>(size, 0);
        for (auto i = 0; i < size; i++) {
            if (i < x.size()) {
                result[i] += x[i];
            }
            if (i < y.size()) {
                if (positive) {
                    result[i] += y[i];
                } else {
                    result[i] -= y[i];
                }
            }
        }
        return result;
    }

    Value operator+(const Value& lhs, const Value& rhs) {
        auto lhs_n = lhs.get_numerator() * rhs.get_denominator();
        auto rhs_n = rhs.get_numerator() * lhs.get_denominator();
        auto den = rhs.get_denominator() * lhs.get_denominator();
        auto num = number::plus(lhs_n, rhs_n, true);
        return Value(num, den);
    }

    Value operator-(const Value& lhs, const Value& rhs) {
        auto lhs_n = lhs.get_numerator() * rhs.get_denominator();
        auto rhs_n = rhs.get_numerator() * lhs.get_denominator();
        auto den = rhs.get_denominator() * lhs.get_denominator();
        auto num = number::plus(lhs_n, rhs_n, false);
        return Value(num, den);
    }

    Value operator*(const Value& lhs, const Value& rhs) {
        auto num = lhs.get_numerator() * rhs.get_numerator();
        auto den = lhs.get_denominator() * rhs.get_denominator();
        return Value(num, den);
    }

    Value operator/(const Value& lhs, const Value& rhs) {
        auto num = lhs.get_numerator() * rhs.get_denominator();
        auto den = lhs.get_denominator() * rhs.get_numerator();
        return Value(num, den);
    }

    Value from_bool(bool b) {
        if (b) {
            return Value{{0, 1}, {1}};
        } else {
            return Value{{}, {1}};
        }
    }

    Value operator&&(const Value& lhs, const Value& rhs) {
        return from_bool(lhs.to_bool() && rhs.to_bool());
    }

    Value operator||(const Value& lhs, const Value& rhs) {
        return from_bool(lhs.to_bool() || rhs.to_bool());
    }

    bool equal(const Value& lhs, const Value& rhs) {
        auto lhs_n = lhs.get_numerator() * rhs.get_denominator();
        auto rhs_n = rhs.get_numerator() * lhs.get_denominator();

        auto degree = std::max(lhs_n.size(), rhs_n.size());

        for (auto i = 1; i <= degree; i++) {
            auto lhs = BigInt(0);
            if (i < lhs_n.size()) {
                lhs = lhs_n[i];
            }
            auto rhs = BigInt(0);
            if (i < rhs_n.size()) {
                rhs = rhs_n[i];
            }
            if (rhs != lhs) {
                return false;
            }
        }
        return true;
    }

    Value operator==(const Value& lhs, const Value& rhs) { return from_bool(equal(lhs, rhs)); }

    Value operator!=(const Value& lhs, const Value& rhs) { return from_bool(!equal(lhs, rhs)); }

    std::pair<BigInt, BigInt> evaluate_with_margin(const std::vector<BigInt>& v, int sf) {
        assert(sf > 0 && sf <= PI_DIGITS);
        auto result = std::make_pair(BigInt(0), BigInt(0));
        auto pi = BigInt(std::string(PI, PI + sf));
        auto ten = big_pow10(sf - 1);
        auto acc = ten;
        auto add_margin = [&](const BigInt& e) {
            if (e < 0) {
                result.first += e - 1;
                result.second += e;
            } else if (e > 0) {
                result.first += e;
                result.second += e + 1;
            }
        };
        if (!v.empty()) {
            add_margin(v[0] * acc);
        }
        for (auto i = 1; i < v.size(); i++) {
            acc *= pi;
            acc /= ten;
            add_margin(v[i] * acc);
        }
        return result;
    }

    BigInt evaluate(const std::vector<BigInt>& v, int sf) {
        assert(sf > 0 && sf <= PI_DIGITS);
        auto result = BigInt(0);
        auto pi = BigInt(std::string(PI, PI + sf));
        auto ten = big_pow10(sf - 1);
        auto acc = ten;
        if (!v.empty()) {
            result += v[0] * acc;
        }
        for (auto i = 1; i < v.size(); i++) {
            acc *= pi;
            acc /= ten;
            result += v[i] * acc;
        }
        return result;
    }

    std::optional<bool> less_than(const Value& lhs, const Value& rhs, int sf) {
        auto lhs_n = evaluate_with_margin(lhs.get_numerator(), sf);
        auto lhs_d = evaluate_with_margin(lhs.get_denominator(), sf);
        if (lhs_d.first < 0) {
            lhs_n.first = -lhs_n.second;
            lhs_n.second = -lhs_n.first;
            lhs_d.first = -lhs_d.second;
            lhs_d.second = -lhs_d.first;
        }
        auto rhs_n = evaluate_with_margin(rhs.get_numerator(), sf);
        auto rhs_d = evaluate_with_margin(rhs.get_denominator(), sf);
        if (rhs_d.first < 0) {
            rhs_n.first = -rhs_n.second;
            rhs_n.second = -rhs_n.first;
            rhs_d.first = -rhs_d.second;
            rhs_d.second = -rhs_d.first;
        }
        if (lhs_n.second * rhs_d.second < rhs_n.first * lhs_d.first) {
            return true;
        }
        if (lhs_n.first * rhs_d.first > rhs_n.second * lhs_d.second) {
            return false;
        }
        return std::nullopt;
    }

    bool less_than(const Value& lhs, const Value& rhs) {
        if (equal(lhs, rhs)) {
            return false;
        }
        auto sf = 8;
        // NOLINTBEGIN(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
        auto max_degree = std::max<int>(lhs.get_numerator().size(), lhs.get_denominator().size());
        max_degree = std::max<int>(max_degree, rhs.get_numerator().size());
        max_degree = std::max<int>(max_degree, rhs.get_denominator().size());
        // NOLINTEND(cppcoreguidelines-narrowing-conversions, bugprone-narrowing-conversions)
        max_degree--;
        while (sf <= PI_DIGITS) {
            auto lt = less_than(lhs, rhs, sf);
            if (lt) {
                return *lt;
            }
            sf += std::floor(std::log2(PI_DIGITS) - std::log2(sf) + 1);
        }
        std::cout << "Not enough pi digits to figure out which if " << lhs.to_string() << " < "
                  << rhs.to_string() << '\n';
        return false;
    }

    Value operator<(const Value& lhs, const Value& rhs) {
        auto lt = less_than(lhs, rhs);
        return from_bool(lt);
    }

    Value operator>=(const Value& lhs, const Value& rhs) {
        auto lt = less_than(lhs, rhs);
        return from_bool(!lt);
    }

    Value operator>(const Value& lhs, const Value& rhs) {
        // NOLINTNEXTLINE(readability-suspicious-call-argument)
        auto lt = less_than(rhs, lhs);
        return from_bool(lt);
    }

    Value operator<=(const Value& lhs, const Value& rhs) {
        // NOLINTNEXTLINE(readability-suspicious-call-argument)
        auto lt = less_than(rhs, lhs);
        return from_bool(!lt);
    }

    Value operator!(const Value& lhs) { return from_bool(!lhs.to_bool()); }

    BigInt substitute(const std::vector<BigInt>& v, const BigInt& pi) {
        auto result = BigInt(0);
        auto acc = BigInt(1);
        if (!v.empty()) {
            result += v[0] * acc;
        }
        for (auto i = 1; i < v.size(); i++) {
            acc *= pi;
            result += v[i] * acc;
        }
        return result;
    }

    std::size_t hash(const Value& value, int length) {
        auto pi = 314159;
        auto num = substitute(value.get_numerator(), pi);
        auto den = substitute(value.get_denominator(), pi);
        auto g = gcd(num, den);
        num /= g;
        den /= g;
        if (den < 0) {
            num = -num;
            den = -den;
        }
        num %= den * length * pi;
        auto hasher = std::hash<std::string>();
        return hasher(num.to_string()) ^ hasher(den.to_string());
    }

    template <typename T>
    const T& get_const_ref(const std::variant<T, std::reference_wrapper<const T>>& t) {
        return std::visit(
            [&]<typename T2>(const T2& t) -> const T& {
                static_assert(std::is_same_v<T2, std::decay_t<T2>>);
                // NOLINTNEXTLINE(bugprone-return-const-ref-from-parameter)
                return t;
            },
            t);
    }

    Index::Index(Value&& value, int length, std::size_t hash)
        : m_value{std::move(value)}, m_length{length}, m_hash{hash} {}
    Index::Index(const Value& value, int length)
        : m_value{std::reference_wrapper{value}}, m_length{length} {
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        m_hash = number::hash(get_const_ref(m_value), m_length);
    }

    Index::Index(Value&& value, int length) : m_value{std::move(value)}, m_length{length} {
        // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
        m_hash = number::hash(get_const_ref(m_value), m_length);
    }

    Index Index::make_ref(const Value& value, int length) { return {value, length}; }

    const Value& Index::get_value() const {
        return get_const_ref(m_value);
    }

    Index Index::clone() const {
        return {std::get<Value>(m_value)
                    .clone(), // cloning a const reference index is probably lifetime violation
                m_length, m_hash};
    }

    std::size_t Index::hash() const { return m_hash; }

    bool Index::operator==(const Index& rhs) const {
        assert(m_length == rhs.m_length);
        const auto& lhs_num = get_const_ref(m_value).get_numerator();
        const auto& rhs_num = get_const_ref(rhs.m_value).get_numerator();
        const auto& lhs_den = get_const_ref(m_value).get_denominator();
        const auto& rhs_den = get_const_ref(rhs.m_value).get_denominator();
        auto lhs_nd = lhs_num * rhs_den;
        auto rhs_nd = rhs_num * lhs_den;
        auto den = lhs_den * rhs_den;
        assert(den.size() == 0 || den.back() != 0);

        auto diff = lhs_nd - rhs_nd;
        while (diff.size() > 0 && diff.back() == 0) {
            diff.pop_back();
        }

        if (diff.size() == 0) {
            return true;
        }
        if (diff[0] != 0) {
            return false;
        }
        if (diff.size() != den.size() + 1) {
            return false;
        }
        auto ratio = get_ratio(diff.back(), den.back() * m_length);
        if (!ratio) {
            return false;
        }
        for (int i = 0; i < den.size() - 1; i++) {
            if (ratio != get_ratio(diff[i + 1], den[i] * m_length)) {
                return false;
            }
        }
        return true;
    }
} // namespace number

std::size_t std::hash<number::Index>::operator()(const number::Index& i) const noexcept {
    return i.hash();
}
