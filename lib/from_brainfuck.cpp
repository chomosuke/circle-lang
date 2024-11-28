#include "from_brainfuck.hpp"

#include <sstream>
#include <string>

std::string from_brainfuck(const std::string& src_code) {
    std::stringstream ss{};
    ss << "(S);\n";
    for (const auto& c : src_code) {
        switch (c) {
        case '>':
            ss << "(P) := (P) + 1*1;\n";
            break;
        case '<':
            ss << "(P) := (P) - 1*1;\n";
            break;
        case '+':
            ss << "( (P) ) := ( (P) ) + 1;\n";
            break;
        case '-':
            ss << "( (P) ) := ( (P) ) - 1;\n";
            break;
        case '.':
            ss << "(std_output_char) := ( (P) ) - 1;\n"
                  "(std_output);\n";
            break;
        case ',':
            ss << "(std_input);\n"
                  "( (P) ) := (std_input_char) + 1;\n";
            break;
        case '[':
            ss << "((\n"
                  "( (P) ) - 1;\n";
            break;
        case ']':
            ss << "));\n";
            break;
        default:
            // do nothing
        }
    }
    ss << "(S) := 0;\n";
    return ss.str();
}
