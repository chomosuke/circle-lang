#pragma once

namespace diagnostic {
    // Zero indexed
    struct Position {
        int column{};
        int line{};
    };

    struct Range {
        Position start{};
        Position end{};
    };
} // namespace diagnostic
