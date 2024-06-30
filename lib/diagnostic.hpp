#pragma once

#include <string>
#include <tl/expected.hpp>

namespace diagnostic {
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
    };

    struct Diagnostic {
        Range range;
        std::string message;

        [[nodiscard]] std::string to_string() const;
    };
    std::string to_string(std::vector<Diagnostic> ds);

    template <typename T> using Expected = tl::expected<T, Diagnostic>;
    template <typename T> using ExpectedV = tl::expected<T, std::vector<Diagnostic>>;

    template <typename T> struct WithInfo {
        Range range;
        T t;
    };
} // namespace diagnostic
