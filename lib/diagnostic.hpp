#pragma once

#include <string>

namespace diagnostic {
    // Zero indexed
    struct Position {
        int column;
        int line;
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
