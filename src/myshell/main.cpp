#include <functional>
#include <cstdio> // Readline does not work without this
#include <readline/readline.h>
#include <readline/history.h>
#include "myshell/environment.h"
#include "myshell/builtInCommands.h"
#include <myshell/utilities.h>
#include "myshell/commandsRunning.h"
#include <csignal>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "myshell/Logger.h"


#define MAX_LINE_SIZE 4096
#define MAX_CONNECTIONS 100


int runSimpleShell(bool server, Logger *logger) {
    char *buf;
    std::vector<std::vector<std::string>> pipelineBlocks;
    std::vector<std::map<std::string, std::string>> filesRedirection;
    std::string inputQuery;


    while (true) {
        inputQuery = getVariableValue("PWD") + " $ ";
        if (not server) {
            buf = readline(inputQuery.c_str());
        } else {
            std::cout << inputQuery;
            buf = (char *) malloc(MAX_LINE_SIZE * sizeof(char));
            std::cin.getline(buf, MAX_LINE_SIZE - 1, '\n');
        }
        if (buf == nullptr) break;
        if (strlen(buf) > 0) {
            if (logger != nullptr) {
                logger->log(buf);
            }
            add_history(buf);
        }
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


[[noreturn]] void runServer(int port) {
    std::cout << "Starting server om port " << port << std::endl;
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

    if (bind(sd, (struct sockaddr *) &server, sizeof(server)) == -1) {
        std::cerr << "Could not start the server properly" << std::endl;
        exit(-1);
    }
    listen(sd, MAX_CONNECTIONS);
    while (true) {

        // Get info of sender and create a logger with this info
        char *connectedAddr = (char *) malloc(INET_ADDRSTRLEN);
        struct sockaddr_in addr;
        socklen_t addrLen = sizeof(addr);
        int psd = accept(sd, (struct sockaddr *) &addr, &addrLen);
        int connectedPort = ntohs(addr.sin_port);
        inet_ntop(AF_INET, &addr.sin_addr, connectedAddr, INET_ADDRSTRLEN);
        std::string source = std::string(connectedAddr) + ":" + std::to_string(connectedPort);
        free(connectedAddr);
        Logger logger{source, "log.txt"};


        int pid = fork();
        if (pid < 0) {
            std::cerr << "Error while creating a child process" << std::endl;
            exit(-3);
        }
        if (pid == 0) {


            std::vector<std::vector<std::string>> pipelineBlocks;
            std::vector<std::map<std::string, std::string>> filesRedirection;
            std::string inputQuery;
            std::cout << "New process created" << std::endl;

            if (dup2(psd, 1) != 1) {
                std::cerr << "Error while duplicating the stdout file descriptor" << std::endl;
                exit(EXIT_FAILURE);
            }

            if (dup2(psd, 0) != 0) {
                std::cerr << "Error while duplicating the stdin file descriptor" << std::endl;
                exit(EXIT_FAILURE);
            }
            if (dup2(psd, 2) != 2) {
                std::cerr << "Error while duplicating the stderr file descriptor" << std::endl;
                exit(EXIT_FAILURE);
            }
            runSimpleShell(true, &logger);
            exit(EXIT_SUCCESS);

        } else
            close(psd);
    }
}


int main(int argc, const char **argv) {
    signal(SIGCHLD, SIG_IGN);
    initEnvironmentVariables();
    setVariable("SHELL", argv[0]);
    registerBuildInCommands();
    if (argc >= 2) {
        if (strcmp(argv[1], "--server") == 0) {
            int port = 8000;
            if (argc == 4) {
                if (strcmp(argv[2], "--port") == 0) {
                    std::stringstream portValue(argv[3]);
                    portValue >> port;
                } else {
                    std::cerr << "Wrong arguments" << std::endl;
                    std::cerr << "Usage: $ ./myshell --server [ --port <port_num> ]" << std::endl;
                    return -1;
                }
            } else if (argc != 2) {
                std::cerr << "Wrong number of arguments" << std::endl;
                std::cerr << "Usage: $ ./myshell --server [ --port <port_num> ]" << std::endl;
                return -2;
            }
            runServer(port);
        }
    }
    if (argc > 1) {
        return runInThisInterpreter(argc, argv);
    }
    return runSimpleShell(false, nullptr);

}
