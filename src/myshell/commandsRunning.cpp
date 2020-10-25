#include "myshell/commandsRunning.h"
#include <unistd.h>
#include <wait.h>
#include <map>
#include "myshell/environment.h"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include "myshell/builtInCommands.h"

int runChildProcess(const char *path, char *const *argv, char *const *envp, int inPipeIn, int inPipeOut, int outPipeIn,
                    int outPipeOut, std::map<std::string, std::string>& filesRedirection, bool leave=false) {
    int pid = fork();
    if (pid < 0) {
        std::cerr << "myshell: Could not fork a process" << std::endl;
        return -1;
    }
    if (pid == 0) {
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


        for (auto &redirection: filesRedirection) {
            int fd1, fd2;
            std::string filename = redirection.first;



            if (redirection.second == "0") {
                redirection.second = "&0";
                fd1 = open(redirection.first.c_str(), O_RDWR);
                if (fd1 < 0) {
                    std::cerr << "Error while opening file " << redirection.first << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                std::stringstream ss{filename};
                ss >> fd1;
            }
            if (fd1 == 0 and not (redirection.first.size() == 1 and redirection.first[0] == '0')) {
                exit(EXIT_FAILURE);
            }

            if (redirection.second[0] == '&') {

                redirection.second.erase(0, 1);
                std::stringstream ss2{redirection.second};
                ss2 >> fd2;

                if (fd2 == 0 and redirection.second != "0") {
                    std::cerr << "HERE" << fd2 << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                fd2 = open(redirection.second.c_str(), O_CREAT | O_WRONLY |O_TRUNC);
                if (fd2 < 0) {
                    std::cerr << "Could not open file " << redirection.second;
                    exit(EXIT_FAILURE);
                }
            }
            if (fd2 == 0) std::swap(fd1, fd2);
            if (dup2(fd2, fd1) != fd1) {
                std::cerr << "Error whie redirecting files" << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cout << "HRE";

        }


        execvpe(path, argv, envp);
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
        pipe(p);
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
        setVariable("?", std::to_string(builtInCommands[arguments[0]](processArgs.size() - 1, processArgs.data())));
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