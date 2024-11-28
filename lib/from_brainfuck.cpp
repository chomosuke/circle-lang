#include "from_brainfuck.hpp"
#include "lib/utils.hpp"

#include <sstream>
#include <string>

std::string from_brainfuck(const std::string& src_code) {
    std::stringstream ss{};
    ss << "(S)\n";
    int indent = 0;
    for (const auto& c : src_code) {
        switch (c) {
        case '>':
            print_indent(ss, indent);
            ss << "; (P) := (P) + 1*1\n";
            break;
        case '<':
            print_indent(ss, indent);
            ss << "; (P) := (P) - 1*1\n";
            break;
        case '+':
            print_indent(ss, indent);
            ss << "; ( (P) ) := ( (P) ) + 1\n";
            break;
        case '-':
            print_indent(ss, indent);
            ss << "; ( (P) ) := ( (P) ) - 1\n";
            break;
        case '.':
            print_indent(ss, indent);
            ss << "; (std_output_char) := ( (P) ) - 1\n"
                  "; (std_output)\n";
            break;
        case ',':
            print_indent(ss, indent);
            ss << "; (std_input)\n"
                  "; ( (P) ) := (std_input_char) + 1\n";
            break;
        case '[':
            print_indent(ss, indent);
            ss << "; ((\n";
            indent++;
            print_indent(ss, indent);
            ss << "( (P) ) - 1\n";
            break;
        case ']':
            indent--;
            print_indent(ss, indent);
            ss << "))\n";
            break;
        default:
            // do nothing
        }
    }
    ss << "; (S) := 0\n";
    return ss.str();
}
