#ifndef MYSHELL_UTILS_H
#define MYSHELL_UTILS_H


#include <string>
#include <iterator>
#include <sstream>
#include <glob.h>
#include <vector>
#include <map>


int replaceDescriptors(std::map<std::string, std::string> &filesRedirection, std::map<int, int>& fdsMapping);

int restoreFileDescriptors(std::map<int, int>& fdsMapping);

int
preProcessLine(std::string &&line, std::vector<std::vector<std::string>> &pipeLineBlocks,
               std::vector<std::map<std::string, std::string>> &filesRedirection);


#endif //MYSHELL_UTILS_H
