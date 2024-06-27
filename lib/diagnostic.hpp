#pragma once

#include <string>
#include <vector>

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
        std::vector<Range> range;
        std::string message;
    };
} // namespace diagnostic
