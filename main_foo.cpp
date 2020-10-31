#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <unistd.h>
#include <csignal>


const int bufSize = 4096;


int main() {
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    signal(SIGCHLD, SIG_IGN);

    if (sd == -1) {
        std::cerr << "Could not create a socket" << std::endl;
        return -2;
    }
    struct sockaddr_in server{};
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8889);
    server.sin_addr.s_addr = ntohl(INADDR_ANY);

    int res = bind(sd, (struct sockaddr *) &server, sizeof(server));
    if (res == -1) {
        std::cerr << "Could not start the server properly" << std::endl;
        return -1;
    }
    listen(sd, 1u << 16u);
    while (true) {
        int psd = accept(sd, nullptr, nullptr);
        int pid = fork();
        if (pid < 0) {
            std::cerr << "Error while creating a child process" << std::endl;
            break;
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
