#include "environment.h"

#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <iostream>

//std::map<std::string, std::string> globalEnvp;
std::map<std::string, std::string> childEnvp;


void exportVariable(std::string varName) {
//    if (globalEnvp.find(varName) == globalEnvp.end()) {
//        globalEnvp[varName] = "";
//    }
//    childEnvp[varName] = globalEnvp[varName];
    char* varValue = getenv(varName.c_str());
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


std::string getVariableValue(std::string varName) {
    char* varValue = getenv(varName.c_str());
    if (varValue == nullptr) {
        return "";
    }
    return varValue;
}

char **getEnvpForChild() {
    char **envp = (char **) malloc(sizeof(char *) * childEnvp.size() + 1);
    int variableNum = 0;
    for (auto &s: childEnvp) {
        char *var = (char *) malloc(sizeof(char) * (s.first.size() + s.second.size()));
        memcpy(var, s.first.c_str(), s.first.size());
        var[s.first.size()] = '=';
        memcpy(var + s.first.size() + 1, s.second.c_str(), s.second.size());
        envp[variableNum] = var;
        variableNum++;
        free(var);
    }
    envp[variableNum] = nullptr;
    return envp;
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


void initEnvironmentVariables() {
    setVariable("?", "0");
}