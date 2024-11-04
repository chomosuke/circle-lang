#pragma once

#include <optional>
#include <string>
#include <tl/expected.hpp>

namespace diagnostic {
    // Zero indexed
    struct Position {
        int line;
        int column;

        int operator<=>(const Position& p) const;
        bool operator==(const Position& p) const = default;
    };

    struct Range {
        Position start;
        Position end;

        int operator<=>(const Range& r) const = default;

        [[nodiscard]] std::string to_string() const;
    };

    struct Diagnostic {
        Range range;
        std::string message;

        [[nodiscard]] std::string to_string() const;
    };
    std::string to_string(std::vector<Diagnostic> ds);

    template <typename T> using Expected = tl::expected<T, Diagnostic>;

    class UnexpectedV {
      private:
        std::vector<Diagnostic> m_ds;

      public:
        explicit UnexpectedV(std::vector<Diagnostic>&& ds);

        [[nodiscard]] std::vector<Diagnostic> extract_ds();
    };

    // Allows non-fatal diagnostics to be carried around while continuing compilation
    template <typename T> class ExpectedV {
      private:
        std::vector<Diagnostic> m_ds;
        std::optional<T> m_value;

      public:
        explicit ExpectedV(T&& value) : m_value{std::move(value)} {}
        // NOLINTNEXTLINE(hicpp-explicit-conversions)
        ExpectedV(UnexpectedV&& ue)
            : m_ds{ue.extract_ds()}, m_value{std::nullopt} {}
        ExpectedV(T value, std::vector<Diagnostic>&& ds);

        [[nodiscard]] std::optional<T> extract_value() { return std::move(m_value); }

        [[nodiscard]] std::vector<Diagnostic> extract_ds() { return std::move(m_ds); }
    };

    template <typename T> struct WithInfo {
        Range range;
        T t;
    };
} // namespace diagnostic
