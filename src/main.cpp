#include "lib/config.hpp"
#include "lib/from_brainfuck.hpp"
#include "lib/interpret.hpp"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

namespace o {
    const char* const HELP = "help";
    const char* const DEBUG = "debug";
    const char* const FROM_BF = "from-bf";
    const char* const SRC_FILE = "src-file";
} // namespace o

int main(int argc, char* argv[]) {
    // Declare the supported options.
    po::options_description visible;
    visible.add_options()(o::HELP, "produce this help message");
    visible.add_options()(o::DEBUG, "run script with debugger");
    visible.add_options()(o::FROM_BF,
                          "treat input and Brainfuck code and transpile it to circle-lang");

    std::string src_fname{};

    po::options_description options;
    options.add_options()(o::SRC_FILE, po::value<std::string>(&src_fname),
                          "source file to interpret");
    options.add(visible);

    po::positional_options_description p;
    p.add(o::SRC_FILE, 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).positional(p).run(), vm);
    po::notify(vm);

    if (vm.contains(o::HELP)) {
        std::cout << "Usage: circle-lang <source file>\n" << visible;
        return 1;
    }

    std::ifstream src_file(src_fname);
    if (src_file.fail()) {
        std::cout << "Can not open source file " << src_fname
                  << ", please make sure that the file exist." << '\n';
        return 1;
    }

    std::stringstream src_code_s{};
    src_code_s << src_file.rdbuf();
    std::string src_code = src_code_s.str();

    if (vm.contains(o::FROM_BF)) {
        std::cout << from_brainfuck(src_code);
    } else {
        interpret(src_code, std::cin, std::cout, std::cerr, Config{.debug{vm.contains(o::DEBUG)}});
    }
}
