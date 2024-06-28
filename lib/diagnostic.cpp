#include "diagnostic.hpp"
#include <format>
#include <string>

namespace diagnostic {
    Diagnostic::Diagnostic(Range range, std::string message)
        : m_range{range}, m_message{std::move(message)} {}

    std::string Diagnostic::to_string() {
        return std::format("{}:{}-{}:{}: {}", m_range.start.line, m_range.start.column,
                           m_range.end.line, m_range.end.column, m_message);
    }
} // namespace diagnostic
