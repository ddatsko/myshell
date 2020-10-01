#include "mycat/programOptions/programOptionsParse.h"
#include <iostream>

namespace po = boost::program_options;

int parseCommandLineArguments(CommandLineArguments &args, int argc, char **argv) {
    po::options_description desc{"Description"};
    desc.add_options()
            ("help,h", "Display help info")
            (",A", "Display invisible characters as hex codes")
            ("input-files", po::value<std::vector<std::string> >(), "input files");


    po::positional_options_description p;
    p.add("input-files", -1);
    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).
                options(desc).positional(p).run(), vm);
        po::notify(vm);
    } catch (boost::wrapexcept<boost::program_options::unknown_option> &e) {
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << desc << std::endl;
        return -2;
    }
    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return -1;
    }
    if (vm.count("-A")) {
        args.displayInvisible = true;
    }
    if (vm.count("input-files") == 0) {
        return 0;
    }
    args.inputFiles = vm["input-files"].as<std::vector<std::string>>();
    return 0;
}