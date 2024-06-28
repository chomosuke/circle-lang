#include "diagnostic.hpp"
#include <format>
#include <string>

namespace diagnostic {
    std::string Diagnostic::to_string() {
        return std::format("{}:{}-{}:{}: {}", range.start.line + 1, range.start.column + 1,
                           range.end.line + 1, range.end.column, message);
    }
} // namespace diagnostic
