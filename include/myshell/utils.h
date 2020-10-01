#ifndef MYSHELL_UTILS_H
#define MYSHELL_UTILS_H


#include <string>
#include <iterator>
#include <sstream>
#include <glob.h>
#include <vector>


int preProcessLine(std::string &&line, std::vector<std::string> &res);


#endif //MYSHELL_UTILS_H
