#pragma once

#include <string>

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

} // namespace diagnostic
