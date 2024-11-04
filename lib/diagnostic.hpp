#pragma once

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
    std::string to_string(std::vector<Diagnostic> ds);

    using Diags = std::vector<diag::Diagnostic>;

    template <typename T> struct WithInfo {
        Range range;
        T t;
    };
} // namespace diag
