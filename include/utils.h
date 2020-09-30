#ifndef MYSHELL_UTILS_H
#define MYSHELL_UTILS_H


#include <string>
#include <iterator>
#include <sstream>

std::string getAbsFilename(const std::string &filename);

std::string preProcessString(std::string &line) {
    std::string res;
    std::string escapedCharacters = "~`#$&*()[]{}\\|\"';?!";
    struct {
        bool insideDoubleQuotes = false;
        bool insideSingleQuotes = false;
        bool afterEscape = false;
        std::string curVariable;
    } curState;
    for (auto &c: line) {
        if (c == '\'') {

    }
}


template<class Container>
void split(const std::string &str, Container &cont) {
    std::istringstream iss(str);
    std::copy(std::istream_iterator<std::string>(iss),
              std::istream_iterator<std::string>(),
              std::back_inserter(cont));
}


#endif //MYSHELL_UTILS_H
