#include "myshell/utils.h"
#include <myshell/environment.h>
#include <glob.h>
#include <map>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>


int isValidFd(int fd) {
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}


int replaceDescriptors(std::map<std::string, std::string> &filesRedirection, std::map<int, int> &fdsMapping) {
    for (auto &redirection: filesRedirection) {
        int fd1, fd2;
        std::string filename = redirection.first;


        if (redirection.second == "0") {
            redirection.second = "&0";
            fd1 = open(redirection.first.c_str(), O_RDWR);
            if (fd1 < 0) {
                std::cerr << "Error while opening file " << redirection.first << std::endl;
                return -1;
            }
        } else {
            std::stringstream ss{filename};
            ss >> fd1;
        }
        if (fd1 == 0 and not(redirection.first.size() == 1 and redirection.first[0] == '0')) {
            return -1;
        }

        if (redirection.second[0] == '&') {

            redirection.second.erase(0, 1);
            std::stringstream ss2{redirection.second};
            ss2 >> fd2;

            if (fd2 == 0 and redirection.second != "0") {
                return -1;
            }
        } else {
            fd2 = open(redirection.second.c_str(), O_CREAT | O_WRONLY | O_TRUNC);
            if (fd2 < 0) {
                std::cerr << "Could not open file " << redirection.second;
                return -1;
            }
        }
        if (fd2 == 0) std::swap(fd1, fd2);

        int reservedFd;
        bool replaced;
        if (isValidFd(fd1)) {
            reservedFd = dup(fd1);
            replaced = true;
        }
        if (dup2(fd2, fd1) != fd1) {
            std::cerr << "Error whie redirecting files" << std::endl;
            return -1;
        }
        if (replaced) {
            fdsMapping[reservedFd] = fd1;

        }

    }
    return 0;

}

int restoreFileDescriptors(std::map<int, int> &fdsMapping) {
    for (auto &descriptors: fdsMapping) {
        dup2(descriptors.first, descriptors.second);
    }
    return 0;
}


struct parseState {
    bool insideDoubleQuotes = false;
    bool insideSingleQuotes = false;
    bool afterEscape = false;
    bool inVariable = false;
    bool openVarBracer = false;
    std::string curVariable;
    std::string curRedirection;
    std::map<std::string, std::string> filesRedirection;
    char redirectionSign = '>';
};


int calculateGlob(std::string &pattern, std::vector<std::string> &res) {
    glob_t globBuf;
    if (glob(pattern.c_str(), 0, nullptr, &globBuf) != 0) {
        return -1;
    }
    for (int i = 0; i < globBuf.gl_pathc; i++) {
        res.emplace_back(globBuf.gl_pathv[i]);
    }
    globfree(&globBuf);
    return 0;
}


void breakArgument(parseState &curState, std::string &currentArgument, std::vector<std::string> &res) {
    if (curState.inVariable) {
        currentArgument += getVariableValue(curState.curVariable);
        curState.curVariable.clear();
        curState.inVariable = false;
    }
}

void addArgument(parseState &curState, std::string &currentArgument, std::vector<std::string> &res) {
    if (not currentArgument.empty()) {
        if (not curState.curRedirection.empty()) {
            if (curState.redirectionSign == '>')
                if (curState.curRedirection == "&") {
                    curState.filesRedirection["1"] = currentArgument;
                    curState.filesRedirection["2"] = "&1";
                } else {
                    curState.filesRedirection[curState.curRedirection] = currentArgument;
                }
            else
                curState.filesRedirection[currentArgument] = curState.curRedirection;
            curState.curRedirection.clear();
            currentArgument.clear();
            return;
        }
        if (calculateGlob(currentArgument, res) != 0) {
            res.push_back(currentArgument);
        }
        currentArgument.clear();
    }
}

int
preProcessLine(std::string &&line, std::vector<std::vector<std::string>> &pipeLineBlocks,
               std::vector<std::map<std::string, std::string>> &filesRedirection) {
    const std::string variableStop = "*?[]/";
    std::string currentArgument;
    struct parseState curState;

    std::vector<std::string> res;
    line.push_back(' '); // Just for convenience
    res.clear();

    for (auto &c: line) {
        if (c == '\\' && not curState.afterEscape) {
            curState.afterEscape = true;
            continue;
        }
        if (curState.openVarBracer) {
            curState.curVariable += c;
            if (curState.insideSingleQuotes and c == '\'') {
                curState.insideSingleQuotes = false;
                continue;
            }
            if (curState.insideDoubleQuotes and c == '"') {
                curState.insideDoubleQuotes = false;
                continue;
            }
            if (c == '\'') {
                curState.insideSingleQuotes = true;
                continue;
            }
            if (c == '"') {
                curState.insideDoubleQuotes = true;
                continue;
            }
            if (curState.insideDoubleQuotes or curState.insideSingleQuotes) continue;

            if (c == ')') {
                curState.openVarBracer = false;
                currentArgument += getVariableValue(curState.curVariable);
                curState.curVariable.clear();
            }
            continue;
        }

        if (curState.afterEscape) {
            curState.afterEscape = false;
            if (curState.inVariable)
                curState.curVariable.push_back(c);
            else
                currentArgument.push_back(c);
            continue;
        }
        if (c == '#' && not curState.insideDoubleQuotes && not curState.insideSingleQuotes) {
            breakArgument(curState, currentArgument, res);
            break;
        }
        if (variableStop.find(c) != std::string::npos) {
            // A glob charavter
            if (curState.inVariable and not curState.openVarBracer) {
                currentArgument += getVariableValue(curState.curVariable);
                curState.curVariable.clear();
            }
            currentArgument.push_back(c);
            continue;
        }
        if (curState.insideSingleQuotes) {
            if (c == '\'') {
                curState.insideSingleQuotes = false;
                res.push_back(currentArgument);
                currentArgument.clear();
                continue;
            }
            currentArgument.push_back(c);
            continue;
        }

        if (curState.insideDoubleQuotes && c == '"') {
            curState.insideDoubleQuotes = false;
            if (not currentArgument.empty()) {
                if (not curState.curRedirection.empty()) {
                    curState.filesRedirection[curState.curVariable] = currentArgument;
                    curState.curRedirection.clear();
                    currentArgument.clear();
                    continue;
                }
                breakArgument(curState, currentArgument, res);
                res.push_back(currentArgument);
                currentArgument.clear();
            }

            continue;
        }

        if (curState.insideDoubleQuotes && isspace(c)) {
            breakArgument(curState, currentArgument, res);
            currentArgument.push_back(c);
            continue;
        }


        if (not curState.insideDoubleQuotes && isspace(c)) {
            breakArgument(curState, currentArgument, res);
            addArgument(curState, currentArgument, res);
            continue;
        }
        if (not curState.insideDoubleQuotes and not curState.insideSingleQuotes and c == '\'') {
            curState.insideSingleQuotes = true;
            continue;
        }

        if (not curState.insideDoubleQuotes and not curState.insideSingleQuotes and c == '\"') {
            curState.insideDoubleQuotes = true;
            continue;
        }
        if (not curState.insideDoubleQuotes and not curState.insideSingleQuotes and
            c == '|') {
            breakArgument(curState, currentArgument, res);
            addArgument(curState, currentArgument, res);
            if (not curState.curRedirection.empty()) {
                std::cerr << "Parsing error" << std::endl;
                return -1;
            }
            filesRedirection.push_back(curState.filesRedirection);
            curState.filesRedirection.clear();
            pipeLineBlocks.push_back(res);
            res.clear();
            continue;
        }
        if (not curState.insideSingleQuotes and not curState.insideDoubleQuotes) {
            if (c == '<') {
                breakArgument(curState, currentArgument, res);
                addArgument(curState, currentArgument, res);
                curState.curRedirection = "0";
                curState.redirectionSign = '<';
                continue;
            } else if (c == '>') {
                if (currentArgument.empty())
                    currentArgument = "1";
                breakArgument(curState, currentArgument, res);
                addArgument(curState, currentArgument, res);
                if (res.empty())
                    return -1;
                curState.curRedirection = res[res.size() - 1];
                curState.redirectionSign = '>';
                res.pop_back();
                continue;
            }

        }

        if (c == '$') {
            if (curState.inVariable) {
                currentArgument += getVariableValue(curState.curVariable);
            }
            curState.curVariable.clear();
            curState.inVariable = true;
            continue;
        }
        if (curState.inVariable) {
            if (c == '(' and curState.curVariable.empty()) {
                curState.openVarBracer = true;
            }
            curState.curVariable.push_back(c);
        } else {
            currentArgument.push_back(c);
        }
    }

    if (curState.openVarBracer or curState.insideDoubleQuotes or curState.insideSingleQuotes) {
        std::cerr << "Error while parsing line. Not close scope" << std::endl;
        return -1;
    }

    if (not curState.curVariable.empty()) {
        currentArgument += getVariableValue(curState.curVariable);
    }
    if (not currentArgument.empty()) {
        breakArgument(curState, currentArgument, res);
        addArgument(curState, currentArgument, res);
    }
//
//    // Check if the path is wsomething like $ ./<name>.msh



    if (not res.empty()) {
        filesRedirection.push_back(curState.filesRedirection);
        pipeLineBlocks.emplace_back(res);
    }

    for (auto &block: pipeLineBlocks) {
        if (block[0].size() >= 7) {
            if (block[0].substr(0, 2) == "./" && block[0].substr(block[0].size() - 4) == ".msh") {
                std::vector<std::string> newBlock;
                newBlock.emplace_back("myshell");
                newBlock.emplace_back(block[0].substr(2));
                for (int i = 1; i < block.size(); i++) {
                    newBlock.push_back(block[i]);
                }
                block = newBlock;
            }
        }
    }


    return 0;
}



