#pragma once

#include "macros.hpp"
#include "number.hpp"
#include <cstdint>
#include <string>
#include <tl/expected.hpp>

namespace diag {
    // Zero indexed
    struct Position {
        int line;
        int column;

        int operator<=>(const Position& p) const;
        bool operator==(const Position& p) const = default;
    };

    struct Range {
        Position start;
        Position end;

        int operator<=>(const Range& r) const = default;

        [[nodiscard]] std::string to_string() const;
    };

    enum Level : std::uint8_t {
        error,
        warning,
    };

    struct Diagnostic {
        Level level;
        Range range;
        std::string message;

        [[nodiscard]] std::string to_string() const;
    };

    class Diags {
      private:
        bool m_fatal{};
        std::vector<Diagnostic> m_diags;

      public:
        Diags() = default;
        void insert(Diagnostic&& diag);
        [[nodiscard]] bool empty() const;
        [[nodiscard]] bool has_fatal() const;
        [[nodiscard]] std::string to_string();
    };

    std::string to_string(number::op::Binary op);
    std::string to_string(number::op::Unary op);
    std::string to_string(const number::Value& v);

    template <typename T> struct WithInfo {
        Range range;
        T t;
    };

    template <typename T1, typename T2> WithInfo<T2> convert_with_info(WithInfo<T1>&& t) {
        return convert_with_info<T1, T2>(std::move(t), [](T1&& t) { return std::move(t); });
    }

    template <typename T1, typename T2>
    WithInfo<T2> convert_with_info(WithInfo<T1>&& t, std::function<T2(T1&&)> convertor) {
        return {.range{t.range}, .t{convertor(std::move(t.t))}};
    }
} // namespace diag
