#pragma once

#include <ostream>

inline void print_indent(std::ostream& ss, int indent) {
    for (auto i = 0; i < indent; i++) {
        ss << "    ";
    }
}
