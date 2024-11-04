#include "diagnostic.hpp"
#include <algorithm>
#include <format>
#include <sstream>
#include <string>

namespace diag {
    int Position::operator<=>(const Position& p) const {
        if (line != p.line) {
            return line - p.line;
        } else {
            return column - p.column;
        }
    }

    std::string Diagnostic::to_string() const {
        return std::format("{}: {}", range.to_string(), message);
    }

    std::string Range::to_string() const {
        return std::format("{}:{}-{}:{}", start.line + 1, start.column + 1, end.line + 1,
                           end.column);
    }

    std::string to_string(std::vector<Diagnostic> ds) {
        std::ranges::sort(ds, [](const Diagnostic& a, const Diagnostic& b) {
            return a.range.start < b.range.start;
        });
        std::stringstream ss;
        for (const auto& d : ds) {
            ss << d.to_string() << '\n';
        }
        return ss.str();
    }
} // namespace diag
