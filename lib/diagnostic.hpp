#pragma once

#include <string>

namespace diagnostic {
    // Zero indexed
    struct Position {
        int line;
        int column;
    };
    bool operator==(const Position& a, const Position& b);

    struct Range {
        Position start;
        Position end;
    };
    bool operator==(const Range& a, const Range& b);

    class Diagnostic {
      private:
        Range m_range;
        std::string m_message;

      public:
        Diagnostic(Range range, std::string message);
        std::string to_string();
    };

} // namespace diagnostic
