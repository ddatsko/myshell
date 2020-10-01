#include "myshell/commandsRunning.h"
#include <unistd.h>
#include <wait.h>
#include <map>
#include "myshell/environment.h"
#include <iostream>
#include "myshell/builtInCommands.h"

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


void processInputLine(const std::vector<std::string> &arguments) {
    std::vector<const char *> processArgs;

    // Get all arguments for execution
    if (arguments.empty()) return;

    // Check if the command is the assignment one
    int equalSignPos;
    if (arguments[0][0] != '=' && (equalSignPos = arguments[0].find('=')) != std::string::npos) {
        setVariable(arguments[0].substr(0, equalSignPos), arguments[0].substr(equalSignPos + 1));
        return;
    }

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
        std::string arg = arguments[0];
        char **envp = getEnvpForChild();
        runChildProcess(arg.c_str(), const_cast<char **>(processArgs.data()), envp);
        freeEnvp(envp);
    }
}