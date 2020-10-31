#include <functional>
#include <cstdio> // Readline does not work without this
#include <readline/readline.h>
#include <readline/history.h>
#include "myshell/environment.h"
#include "myshell/builtInCommands.h"
#include <myshell/utils.h>
#include "myshell/commandsRunning.h"
#include <csignal>



int main(int argc, const char **argv) {
    signal(SIGCHLD,SIG_IGN);
    initEnvironmentVariables();
    setVariable("SHELL", argv[0]);
    registerBuildInCommands();
    char *buf;
    std::vector<std::vector<std::string>> pipelineBlocks;
    std::vector<std::map<std::string, std::string>> filesRedirection;
    std::string inputQuery;

    if (argc > 1) {
        return runInThisInterpreter(argc, argv);
    }

    while (true) {
        inputQuery = getVariableValue("PWD") + " $ ";
        buf = readline(inputQuery.c_str());
        if (buf == nullptr) break;
        if (strlen(buf) > 0)
            add_history(buf);
        if (preProcessLine(buf, pipelineBlocks, filesRedirection) != 0) {
            free(buf);
            continue;
        }

        processInputLine(pipelineBlocks, filesRedirection);
        pipelineBlocks.clear();
        filesRedirection.clear();
        free(buf);
    }
}