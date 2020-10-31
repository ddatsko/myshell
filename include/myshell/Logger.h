#ifndef MYSHELL_LOGGER_H
#define MYSHELL_LOGGER_H

#include <string>

class Logger {
private:
    std::string source;
    std::string outFileName;
public:

    Logger(std::string &source, std::string &&outFileName);

    void log(std::string &&message);
};


#endif //MYSHELL_LOGGER_H
