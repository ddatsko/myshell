#include <ctime>
#include <zconf.h>
#include "myshell/Logger.h"
#include <fcntl.h>
#include "myshell/utilities.h"

Logger::Logger(std::string &source, std::string &&outFileName) {
    this->source = source;
    this->outFileName = outFileName;
}

void Logger::log(std::string &&message) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S ", timeinfo);
    int tmp = open(outFileName.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0666);
    std::string toPrint = std::string("[info] ") + buffer + source + " " + message;
    if (tmp >= 0)
        writeBuf(tmp, toPrint.c_str(), toPrint.size());
    close(tmp);
}
