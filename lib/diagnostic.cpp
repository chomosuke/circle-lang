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

    void Diags::insert(Diagnostic&& diag) {
        if (diag.level == error) {
            m_fatal = true;
        }
        m_diags.push_back(std::move(diag));
    }

    bool Diags::empty() const { return m_diags.empty(); }

    bool Diags::has_fatal() const { return m_fatal; }

    std::string Diags::to_string() {
        std::ranges::sort(m_diags, [](const Diagnostic& a, const Diagnostic& b) {
            return a.range.start < b.range.start;
        });
        std::stringstream ss;
        for (const auto& d : m_diags) {
            switch (d.level) {
            case error:
                ss << "[ERROR] ";
                break;
            case warning:
                ss << "[WARNING] ";
                break;
            }
            ss << d.to_string() << '\n';
        }
        return ss.str();
    }

    std::string to_string(number::op::Binary op) {
        switch (op) {
        case number::op::plus:
            return "+";
        case number::op::minus:
            return "-";
        case number::op::multiply:
            return "*";
        case number::op::divide:
            return "/";
        case number::op::bool_and:
            return "&&";
        case number::op::bool_or:
            return "||";
        case number::op::equal:
            return "=";
        case number::op::not_equal:
            return "!=";
        case number::op::smaller:
            return "<";
        case number::op::smaller_or_equal:
            return "<=";
        case number::op::greater:
            return ">";
        case number::op::greater_or_equal:
            return ">=";
        default:
            assert(false && "Operator not found");
        }
    }
    std::string to_string(number::op::Unary op) {
        switch (op) {
        case number::op::bool_not:
            return "!";
        default:
            assert(false && "Operator not found");
        }
    }
    std::string to_string(const number::Value& v) {
        auto letters = v.to_letters();
        if (letters) {
            return *letters;
        }

        return v.to_string();
    }

    std::string Range::to_string() const {
        return std::format("{}:{}-{}:{}", start.line + 1, start.column + 1, end.line + 1,
                           end.column);
    }
} // namespace diag
