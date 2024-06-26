#include "lib/interpret.hpp"
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace po = boost::program_options;

namespace o {
    constexpr char help[]{"help"};
    constexpr char src_file[]{"src-file"};
} // namespace o

int main(int argc, char* argv[]) {
    // Declare the supported options.
    po::options_description visible;
    visible.add_options()(o::help, "produce this help message");

    std::string src_fname{};

    po::options_description options;
    options.add_options()(o::src_file, po::value<std::string>(&src_fname),
                          "source file to interpret");
    options.add(visible);

    po::positional_options_description p;
    p.add(o::src_file, 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(options).positional(p).run(), vm);
    po::notify(vm);

    if (vm.count(o::help) > 0 || src_fname == "") {
        std::cout << "Usage: circle-lang <source file>\n" << visible;
        return 1;
    }

    std::ifstream src_file(src_fname);
    if (src_file.fail()) {
        std::cout << "Can not open source file " << src_fname
                  << ", please make sure that the file exist." << std::endl;
        return 1;
    }

    std::stringstream src_code_s{};
    src_code_s << src_file.rdbuf();
    std::string src_code = src_code_s.str();

    interpret(src_code, std::cin, std::cout, std::cerr);
}
