#include "number.hpp"
#include <climits>
#include <sstream>
#include <iterator>

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

        bool operator==(const RotateableIndex& rhs) const {
            return m_t == rhs.m_t && m_offset == rhs.m_offset;
        }

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

    const std::vector<BigInt>& Value::get_numerator() { return m_numerator; }

    const std::vector<BigInt>& Value::get_denominator() { return m_denominator; }

    std::optional<std::string> Value::to_letters() {
        std::stringstream ss{};
        BigInt base{1};
        for (const auto& n : m_numerator) {
            if (n % base != 0 || n / base > CHAR_MAX) {
                return std::nullopt;
            }
            ss << static_cast<char>((n / base).to_int());
            base *= number::LETTER_BASE;
        }
        return ss.str();
    }
} // namespace number
