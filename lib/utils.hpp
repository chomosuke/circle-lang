#pragma once

#include <ostream>
#include <variant>

template <class... Args> struct VariantCastProxy {
    std::variant<Args...> v;

    // NOLINTNEXTLINE(hicpp-explicit-conversions)
    template <class... ToArgs> operator std::variant<ToArgs...>() {
        return std::visit([](auto&& arg) -> std::variant<ToArgs...> { return arg; }, std::move(v));
    }
};

template <class... Args> auto variant_cast(std::variant<Args...>&& v) -> VariantCastProxy<Args...> {
    return {std::move(v)};
}

inline void print_indent(std::ostream& ss, int indent) {
    for (auto i = 0; i < indent; i++) {
        ss << "    ";
    }
}
