
#ifndef MYSHELL_COMMANDSRUNNING_H
#define MYSHELL_COMMANDSRUNNING_H

#include <vector>
#include <string>
#include <map>


int
processOneBlock(std::vector<std::string> &arguments, int inPipeIn, int inPipeOut, int outPipeIn, int outPipeOut, std::map<std::string, std::string> &filesRedirection);

void runChildProcess(const char *path, char *const *argv, char *const *envp);

void processInputLine(std::vector<std::vector<std::string>> &pipeLineBlocks,
                      std::vector<std::map<std::string, std::string>> &filesRedirection);
#endif //MYSHELL_COMMANDSRUNNING_H
