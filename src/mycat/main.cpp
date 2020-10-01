#include <iostream>
#include <boost/program_options.hpp>
#include <fcntl.h>
#include "mycat/io/io.h"
#include "mycat/programOptions/programOptionsParse.h"

namespace po = boost::program_options;


void closeFiles(std::vector<int> &descriptors) {
    for (int &descriptor : descriptors) {
        close(descriptor);
    }
}

int myCat(CommandLineArguments &args) {
    int fd;
    std::vector<int> fileDescriptors;
    for (auto &file: args.inputFiles) {
        if ((fd = open(file.c_str(), O_RDONLY)) < 0) {
            int stat = errno;
            closeFiles(fileDescriptors);
            std::cout << "Error. File " << file << " does not exist" << std::endl;
            return stat;
        }
        fileDescriptors.push_back(fd);
    }
    for (auto &descriptor : fileDescriptors) {
        if (readPrintFile(descriptor, args.displayInvisible) != 0) {
            std::cout << "Error while processing file with fd " << fd << std::endl;
            return -1;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    CommandLineArguments args;
    if (parseCommandLineArguments(args, argc, argv) == 0)
        return myCat(args);
    return 0;
}
