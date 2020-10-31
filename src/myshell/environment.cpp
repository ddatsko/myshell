#include "myshell/environment.h"

#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <myshell/utilities.h>
#include "myshell/commandsRunning.h"
#include <filesystem>
#include <fcntl.h>

#define MAX_VAR_LINE 4096

std::map<std::string, std::string> childEnvp;
char tempFileTemplate[] = "/tmp/myshellXXXXXXXXXX";


void exportVariable(std::string varName) {
    char *varValue = getenv(varName.c_str());
    if (varValue == nullptr) {
        setenv(varName.c_str(), "", true);
        childEnvp[varName] = "";
    } else {
        childEnvp[varName] = std::string(varValue);
    }


}

void setVariable(std::string varName, std::string varValue) {
    if (childEnvp.find(varName) != childEnvp.end()) {
        childEnvp[varName] = varValue;
    }
    setenv(varName.c_str(), varValue.c_str(), true);
}

void setPwd() {
    char *curDir = get_current_dir_name();
    setVariable("PWD", std::string(curDir));
    free(curDir);
}


ssize_t readBuf(int fd, char *buf, ssize_t size, int *status) {
    ssize_t readBytes = 0;

    if (buf == nullptr || status == nullptr) return -1;
    while (readBytes < size) {
        ssize_t nRead = read(fd, buf + readBytes, size - readBytes);
        if (nRead == -1) {
            if (errno == EINTR) continue;
            *status = errno;
            return -1;
        }
        if (nRead == 0) {
            return readBytes;
        }
        readBytes += nRead;
    }
    return readBytes;
}

char buf[MAX_VAR_LINE];

std::string getVariableValue(std::string varName) {
    if (!varName.empty()) {
        if (varName[0] == '(' and varName[varName.size() - 1] == ')') {
            varName.erase(0, 1);
            varName.erase(varName.size() - 1, 1);
            std::vector<std::vector<std::string>> pipelineBlocks;
            std::vector<std::map<std::string, std::string>> filesRedirection;
            preProcessLine(varName + "", pipelineBlocks, filesRedirection);

            int temp = mkstemp(tempFileTemplate);


            int reserveFd = dup(1);
            if (dup2(temp, 1) != 1) {
                std::cerr << "Error while redirecting output" << std::endl;
            }
            processInputLine(pipelineBlocks, filesRedirection);
            if (dup2(reserveFd, 1) != 1) {
                std::cerr << "Error while redirecting output" << std::endl;
            }
            close(reserveFd);
            lseek(temp, 0, 0);

            int status;
            readBuf(temp, buf, MAX_VAR_LINE - 1, &status);
            close(temp);
            return std::string(buf);
        }
    }
    char *varValue = getenv(varName.c_str());
    if (varValue == nullptr) {
        return "";
    }
    return varValue;
}


std::pair<std::string, std::string> splitByEqualSign(const char *str) {
    std::pair<std::string, std::string> res;
    bool varName = true;
    while (*str != '\0') {
        if (*str == '=' && varName) {
            varName = false;
            str++;
            continue;
        }
        if (varName) {
            res.first.push_back(*str);
        } else {
            res.second.push_back(*str);
        }
        str++;
    }
    return res;
}

void freeEnvp(char **envp) {
    int i = 0;
    while (envp[i] != nullptr) {
        free(envp[i]);
        i++;
    }
    free(envp);
}

char **getEnvpForChild() {
    char **envp = (char **) malloc(sizeof(char *) * childEnvp.size() + 1);
    int variableNum = 0;
    for (auto &s: childEnvp) {
        char *var = (char *) malloc(sizeof(char) * (s.first.size() + s.second.size() + 1));
        memcpy(var, s.first.c_str(), s.first.size());
        var[s.first.size()] = '=';
        memcpy(var + s.first.size() + 1, s.second.c_str(), s.second.size());
        envp[variableNum] = var;
        variableNum++;
    }
    envp[variableNum] = nullptr;
    return envp;
}


void initEnvironmentVariables() {
    char **variable = environ;
    while (*variable != nullptr) {
        auto varPair = splitByEqualSign(*variable);
        exportVariable(varPair.first);
        variable++;
    }
    setVariable("?", "0");

    auto path = getVariableValue("PATH");
    path += ":" + getVariableValue("PWD");
    setVariable("PATH", path);
}