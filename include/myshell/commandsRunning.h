
#ifndef MYSHELL_COMMANDSRUNNING_H
#define MYSHELL_COMMANDSRUNNING_H

#include <vector>
#include <string>


void runChildProcess(const char *path, char *const *argv, char *const *envp);

void processInputLine(const std::vector<std::string> &arguments);

#endif //MYSHELL_COMMANDSRUNNING_H
