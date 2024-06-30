#include "diagnostic.hpp"
#include <format>
#include <sstream>
#include <string>

namespace diagnostic {
    int Position::operator<=>(const Position& p) const {
        if (line != p.line) {
            return line - p.line;
        } else {
            return column - p.column;
        }
    }

    std::string Diagnostic::to_string() const {
        return std::format("{}:{}-{}:{}: {}", range.start.line + 1, range.start.column + 1,
                           range.end.line + 1, range.end.column, message);
    }

    std::string to_string(std::vector<Diagnostic> ds) {
        std::sort(ds.begin(), ds.end(), [](const Diagnostic& a, const Diagnostic& b) {
            return a.range.start < b.range.start;
        });
        std::stringstream ss;
        for (const auto& d : ds) {
            ss << d.to_string() << std::endl;
        }
        return ss.str();
    }
} // namespace diagnostic
