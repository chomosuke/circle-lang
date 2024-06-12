#include "stdincludes.hpp"

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace o {
constexpr char help[]{"help"};
constexpr char src_file[]{"src-file"};
} // namespace o

int main(int argc, char* argv[]) {
    // Declare the supported options.
    po::options_description visible;
    visible.add_options()(o::help, "produce this help message");

    po::options_description options;
    options.add_options()(o::src_file, po::value<string>(),
                          "source file to interpret");
    options.add(visible);

    po::positional_options_description p;
    p.add(o::src_file, 1);

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv)
                  .options(options)
                  .positional(p)
                  .run(),
              vm);
    po::notify(vm);

    if (vm.count(o::help) > 0 || vm.count(o::src_file) == 0) {
        cout << "usage: circle-lang <source file>\n" << visible;
        return 1;
    }

}
