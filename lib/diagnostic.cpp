#include "diagnostic.hpp"
#include <format>
#include <string>

namespace diagnostic {
    bool operator==(const Position& a, const Position& b) {
        return a.line == b.line && a.column == b.column;
    }

    bool operator==(const Range& a, const Range& b) { return a.start == b.start && a.end == b.end; }

    Diagnostic::Diagnostic(Range range, std::string message)
        : m_range{range}, m_message{std::move(message)} {}

    std::string Diagnostic::to_string() {
        return std::format("{}:{}-{}:{}: ", m_range.start.line, m_range.start.column,
                           m_range.end.line, m_range.end.column, m_message);
    }
} // namespace diagnostic
