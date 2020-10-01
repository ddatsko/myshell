#include <iostream>
#include "myshell/builtInCommands.h"
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <myshell/utils.h>
#include "myshell/environment.h"
#include "myshell/commandsRunning.h"

std::map<std::string, std::function<int(int, const char **)>> builtInCommands = std::map<std::string, std::function<int(
        int, const char **)>>{};

// Checks whether the argument is -h or --help
bool isHelpOption(const char *arg) {
    return strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0;
}


int mexport(int argc, const char **argv) {
    for (int i = 0; i < argc; i++) {
        if (isHelpOption(argv[i])) {
            std::cout << "usage: mexport [-h | --help] [x1=y1] [x2=y2] ..." << std::endl;
            return 0;
        }
    }
    for (int i = 0; i < argc; i++) {
        auto argument = std::string(argv[i]);
        std::size_t equalSignPos = argument.find('=');
        if (equalSignPos == std::string::npos) {
            exportVariable(argument);
        } else {
            std::string varName = argument.substr(0, equalSignPos);
            std::string varValue = argument.substr(equalSignPos + 1);
            setVariable(varName, varValue);
            exportVariable(varName);
        }
    }
    return 0;
}

int mecho(int argc, const char **argv) {
    if (argc > 1 && isHelpOption(argv[1])) {
        std::cout << "Usage: mecho [ -h | --help ] [ text | $<var_name> ] [ text | $<var_name> ] ..." << std::endl;
        return 0;
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '$') {
            std::string value = getVariableValue(argv[i] + 1);
            if (not value.empty()) {
                std::cout << value << " ";
            }
        } else {
            std::cout << argv[i] << " ";
        }
    }
    std::cout << std::endl;
    return 0;
}

int merrno(int argc, const char **argv) {
    if (argc > 2) {
        std::cout << "merrno: Too many arguments" << std::endl;
        return -1;
    }
    if (argc == 2) {
        if (isHelpOption(argv[1])) {
            std::cout << "Usage: meerno [-h | --help]\nPrint the exit code of the last command" << std::endl;
            return 0;
        }
        std::cout << "meerno: Invalid option " << argv[1] << std::endl;
        return -2;
    }
    std::cout << getVariableValue("?") << std::endl;
    return 0;
}

int mcd(int argc, const char **argv) {
    if (argc > 2) {
        std::cout << "mcd: Too many arguments" << std::endl;
        return -1;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "Usage: mcd [ <path> | -h | --help ]" << std::endl;
            return 0;
        }
        if (chdir(argv[1]) != 0) {
            std::cout << "mcd: Error! Could not change the directory" << std::endl;
            return -2;
        }
        setPwd();

    }
    return 0;

}

int mexit(int argc, const char **argv) {
    if (argc > 2) {
        std::cout << "mexit: Too many arguments" << std::endl;
        return -1;
    }
    std::string status;
    if (argc == 2) {
        if (isHelpOption(argv[1])) {
            std::cout << "Usage: mexit [<>exit_code | -h | --help]" << std::endl;
            return 0;
        }
        status = std::string(argv[1]);
    } else {
        status = std::string("0");
    }
    try {
        exit(static_cast<uint8_t>(std::stol(status)));
    } catch (std::invalid_argument &) {
        std::cout << "Invalid exit status" << std::endl;
        exit(2);
    }
}

int runInThisInterpreter(int argc, const char **argv) {
    if (argc == 1 || argc > 2) {
        std::cout << argv[0] << " : Invalid number of arguments" << std::endl;
        return -1;
    }
    // Save cinbuf for restoring
    auto cinbuf = std::cin.rdbuf();
    std::ifstream in(argv[1]);
    std::cin.rdbuf(in.rdbuf());
    std::string strBuf;

    std::vector<std::string> arguments;
    while (std::getline(std::cin, strBuf)) {
        preProcessLine(strBuf.c_str(), arguments);
        processInputLine(arguments);
    }
    std::cin.rdbuf(cinbuf);
    return 0;
}


int mpwd(int argc, const char **argv) {
    if (argc > 2) {
        std::cout << "mpwd: too many arguments" << std::endl;
        return -1;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
            std::cout << "Usage: mpwd [-h | --help]\n Prints current working directory" << std::endl;
            return 0;
        }
        std::cout << "mpwd: Invalid argument " << argv[1] << std::endl;
        return -2;
    }
    std::cout << getVariableValue("PWD") << std::endl;
    return 0;
}


void registerBuildInCommands() {
    builtInCommands["mexport"] = mexport;
    builtInCommands["mecho"] = mecho;
    builtInCommands["merrno"] = merrno;
    builtInCommands["mpwd"] = mpwd;
    builtInCommands["mcd"] = mcd;
    builtInCommands["mexit"] = mexit;
    builtInCommands["."] = runInThisInterpreter;
}