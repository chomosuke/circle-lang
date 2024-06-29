#pragma once

#include <string>
#include <tl/expected.hpp>

namespace diagnostic {
    // Zero indexed
    struct Position {
        int line;
        int column;

        bool operator==(const Position& p) const = default;
    };

    struct Range {
        Position start;
        Position end;

        bool operator==(const Range& r) const = default;
    };

    struct Diagnostic {
        Range range;
        std::string message;

        std::string to_string();
    };

    template <typename T> using Expected = tl::expected<T, Diagnostic>;

    template <typename T> struct WithInfo {
        Range range;
        T t;
    };
} // namespace diagnostic
