#include "myshell/commandsRunning.h"
#include <unistd.h>
#include <wait.h>
#include <map>
#include "myshell/environment.h"
#include <iostream>
#include <myshell/utilities.h>
#include "myshell/builtInCommands.h"

int runChildProcess(const char *path, char *const *argv, char *const *envp, int inPipeIn, int inPipeOut, int outPipeIn,
                    int outPipeOut, std::map<std::string, std::string>& filesRedirection, bool leave=false) {
    int pid = fork();
    if (pid < 0) {
        std::cerr << "myshell: Could not fork a process" << std::endl;
        return -1;
    }
    if (pid == 0) {
        if (leave) {
            close(0);
            close(1);
            close(2);
        }
        if (inPipeIn != 0)
            close(inPipeIn);
        if (outPipeOut != 1)
            close(outPipeOut);
        if (outPipeIn != 1)
            if (dup2(outPipeIn, 1) != 1) {
                std::cerr << "HERE" << std::endl;
                exit(EXIT_FAILURE);
            }
        if (inPipeOut != 0)
            if (dup2(inPipeOut, 0) != 0)
                exit(EXIT_FAILURE);
        if (inPipeOut != 0)
            close(inPipeOut);
        if (outPipeIn != 1)
            close(outPipeIn);

        std::map<int, int> fdsMapping;
        if (replaceDescriptors(filesRedirection, fdsMapping) != 0) {
            exit(EXIT_FAILURE);
        }


        execvpe(path, argv, envp);
        restoreFileDescriptors(fdsMapping);
        std::cerr << "Error while running child process" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (leave)
        return 0;
    return pid;
}


void processInputLine(std::vector<std::vector<std::string>> &pipeLineBlocks,
                      std::vector<std::map<std::string, std::string>> &filesRedirection){
    std::vector<int> pids; // For getting errors information from shild processes

    if (pipeLineBlocks.empty())
        return;

    int prevP[2] = {0, 0};
    int p[2];


    for (int i = 0; i < (pipeLineBlocks.size() - 1); i++) {
        if (pipe(p) != 0) {
            std::cerr << "Could not create a pipe" << std::endl;
            continue;
        };
        pids.push_back(processOneBlock(pipeLineBlocks[i], prevP[1], prevP[0], p[1], p[0], filesRedirection[i]));
        if (prevP[0] != 0) close(prevP[0]);
        if (prevP[1] != 0) close(prevP[1]);
        prevP[0] = p[0];
        prevP[1] = p[1];
    }
    pids.push_back(processOneBlock(pipeLineBlocks[pipeLineBlocks.size() - 1], prevP[1], prevP[0], 1, 1, filesRedirection[pipeLineBlocks.size() - 1]));
    if (prevP[0] != 0) close(prevP[0]);
    if (prevP[1] != 0) close(prevP[1]);


    for (int pid: pids) {
        if (pid < 0) {
            std::cerr << "Error occured while creating a child process" << std::endl;
            continue;
        }
        if (pid == 0) {
            continue;
        }
        if (pid > 0) {
            waitpid(pid, nullptr, 0);
        }
    }
}

int
processOneBlock(std::vector<std::string> &arguments, int inPipeIn, int inPipeOut, int outPipeIn, int outPipeOut, std::map<std::string, std::string> &filesRedirection) {
    std::vector<const char *> processArgs;

    // Get all arguments for execution
    if (arguments.empty()) return 0;

    // Check if the command is the assignment one
    int equalSignPos;
    if (arguments[0][0] != '=' && (equalSignPos = arguments[0].find('=')) != std::string::npos) {
        setVariable(arguments[0].substr(0, equalSignPos), arguments[0].substr(equalSignPos + 1));
        return 0;
    }

    bool leave = arguments[arguments.size() - 1] == "&";
    if (leave)
        arguments.pop_back();

    // Convert each argument to const char* from str. (Not include the first one (program name))
    processArgs.reserve(arguments.size());
    for (auto &arg: arguments) {
        processArgs.push_back(arg.c_str());
    }
    // Push null pointer, as gonna use this for argv
    processArgs.push_back(nullptr);


    if (builtInCommands.find(arguments[0]) != builtInCommands.end()) {
        std::map<int, int> fdsMapping;
        replaceDescriptors(filesRedirection, fdsMapping);
        setVariable("?", std::to_string(builtInCommands[arguments[0]](processArgs.size() - 1, processArgs.data())));
        restoreFileDescriptors(fdsMapping);
        return 0;
    } else {
        std::string arg = arguments[0];
        char **envp = getEnvpForChild();


        int pid = runChildProcess(arg.c_str(), const_cast<char **>(processArgs.data()), envp, inPipeIn, inPipeOut,
                                  outPipeIn,
                                  outPipeOut, filesRedirection, leave);
        freeEnvp(envp);
        return pid;
    }
}