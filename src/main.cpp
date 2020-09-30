#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <map>
#include <functional>
#include <readline/readline.h>
#include <readline/history.h>
#include "environment.h"
#include "builtInCommands.h"
#include <utils.h>


void runChildProcess(const char *path, char *const *argv, char *const *envp) {
    int pid = fork();
    if (pid < 0) {
        std::cerr << "myshell: Could not fork a process" << std::endl;
        return;
    }
    if (pid == 0) {
        execvpe(path, argv, envp);
    }
    int returnStatus;
    wait(&returnStatus);
    setVariable("?", std::to_string(returnStatus));
    if (returnStatus != 0) {
        std::cout << "Execution of " << path << " failed" << std::endl;
    }
}


void processInputLine(const std::string &line) {
    std::vector<std::string> arguments;
    std::vector<const char *> processArgs;

    // Get all arguments for execution
    split<std::vector<std::string>>(line, arguments);
    if (arguments.empty()) return;

    // Convert each argument to const char* from str. (Not include the first one (program name))
    processArgs.reserve(arguments.size());
    for (auto &arg: arguments) {
        processArgs.push_back(arg.c_str());
    }
    // Push null pointer, as gonna use this for argv
    processArgs.push_back(nullptr);

    if (builtInCommands.find(arguments[0]) != builtInCommands.end()) {
        setVariable("?", std::to_string(builtInCommands[arguments[0]](processArgs.size() - 1, processArgs.data())));
    } else {
        std::string command = getAbsFilename(arguments[0]);

        char **envp = getEnvpForChild();
        runChildProcess(command.c_str(), const_cast<char **>(processArgs.data()), envp);
        free(envp);
    }
}


int main(int argc, char **argv) {
    initEnvironmentVariables();


    std::string inputQuery;
    if (argc > 1) {
        // TODO: Redirect from argv[1] to stdin here
//        std::cout << "HERE";
//        std::ifstream in(argv[1]);
//        cinbuf = std::cin.rdbuf();
//        std::cin.rdbuf(in.rdbuf());
//
//        std::cin.rdbuf(cinbuf);   //reset to standard input again
    }
    char *buf;
    registerBuildInCommands();

    while (true) {
        inputQuery = getVariableValue("PWD") + " $ ";
        buf = readline(inputQuery.c_str());
        if (buf == nullptr) break;
        if (strlen(buf) > 0)
            add_history(buf);
        processInputLine(std::string(buf));
        free(buf);
    }
}