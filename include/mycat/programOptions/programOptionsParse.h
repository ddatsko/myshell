#ifndef MYCAT_PROGRAMOPTIONSPARSE_H
#define MYCAT_PROGRAMOPTIONSPARSE_H

#include <boost/program_options.hpp>


struct CommandLineArguments {
    bool displayInvisible = false;
    std::vector<std::string> inputFiles;
};
int parseCommandLineArguments(CommandLineArguments &args, int argc, char **argv);


#endif
