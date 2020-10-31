#include <functional>
#include <cstdio> // Readline does not work without this
#include <readline/readline.h>
#include <readline/history.h>
#include "myshell/environment.h"
#include "myshell/builtInCommands.h"
#include <myshell/utils.h>
#include "myshell/commandsRunning.h"
#include <csignal>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>


const int bufSize = 4096;


[[noreturn]] void runServer(int port) {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGCHLD, SIG_IGN);

    if (sd == -1) {
        std::cerr << "Could not create a sock"
                     "et" << std::endl;
        exit(-2);
    }
    struct sockaddr_in server{};
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = ntohl(INADDR_ANY);

    int res = bind(sd, (struct sockaddr *) &server, sizeof(server));
    if (res == -1) {
        std::cerr << "Could not start the server properly" << std::endl;
        exit(-1);
    }
    listen(sd, 1u << 16u);
    while (true) {
        int psd = accept(sd, nullptr, nullptr);
        int pid = fork();
        if (pid < 0) {
            std::cerr << "Error while creating a child process" << std::endl;
            exit(-3);
        }
        if (pid == 0) {
            char *buf = (char *) malloc(bufSize * sizeof(char));
            std::cout << "New process created" << std::endl;
            for (;;) {
                int cc = recv(psd, buf, sizeof(buf), 0);
                std::cout << "Received " << cc << " bytes" << std::endl;
                if (cc == 0) {
                    free(buf);
                    std::cout << "END" << std::endl;
                    exit(EXIT_SUCCESS);
                }


                buf[cc] = '\0';
                printf("message received: %s\n", buf);
                std::string a("Hello");
                if (dup2(psd, 1) != 0) {
                    std::cerr << "Error while duplicating the file descriptor" << std::endl;
                    exit(EXIT_FAILURE);
                }


            }
        }
    }
}


int runSimpleShell(int argc, const char **argv) {
    signal(SIGCHLD, SIG_IGN);
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
    return 0;
}


int main(int argc, const char **argv) {
    if (argc >= 2) {
        if (strcmp(argv[1], "--server") == 0) {
            int port = 8000;
            if (argc == 4) {
                if (strcmp(argv[2], "--port") == 0) {
                    std::stringstream portValue(argv[3]);
                    portValue >> port;
                } else {
                    std::cerr << "Wrong arguments" << std::endl;
                    return -1;
                }
            } else if (argc != 2) {
                std::cerr << "Wrong number of arguments" << std::endl;
                return -2;
            }
            runServer(port);
        }
    }
    return runSimpleShell(argc, argv);

}
