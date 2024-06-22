#pragma once

#include <string>

namespace diagnostic {
    // Zero indexed
    struct Position {
        int line;
        int column;
    };

    struct Range {
        Position start;
        Position end;
    };

    struct Diagnostic {
        Range range;
        std::string message;
    };
} // namespace diagnostic
