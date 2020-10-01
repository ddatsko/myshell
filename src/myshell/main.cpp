#include <functional>
#include <cstdio> // Readline does not work without this
#include <readline/readline.h>
#include <readline/history.h>
#include "myshell/environment.h"
#include "myshell/builtInCommands.h"
#include <myshell/utils.h>
#include "myshell/commandsRunning.h"


int main(int argc, const char **argv) {
    initEnvironmentVariables();
    setVariable("SHELL", argv[0]);
    registerBuildInCommands();
    char *buf;
    std::vector<std::string> arguments;
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

        preProcessLine(buf, arguments);
        processInputLine(arguments);
        free(buf);
    }
}