#include "myshell/utils.h"
#include <myshell/environment.h>
#include <glob.h>


struct parseState {
    bool insideDoubleQuotes = false;
    bool insideSingleQuotes = false;
    bool afterEscape = false;
    bool inVariable = false;
    std::string curVariable;
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

int preProcessLine(std::string &&line, std::vector<std::string> &res) {
    const std::string variableStop = "*?[]/";
    std::string currentArgument;
    struct parseState curState;

    line.push_back(' '); // Just for convenience
    res.clear();

    for (auto &c: line) {
        if (c == '\\' && not curState.afterEscape) {
            curState.afterEscape = true;
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
            if (curState.inVariable) {
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
            if (not currentArgument.empty()) {
                if (calculateGlob(currentArgument, res) != 0) {
                    res.push_back(currentArgument);
                }
                currentArgument.clear();
            }
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

        if (c == '$') {
            if (curState.inVariable) {
                currentArgument += getVariableValue(curState.curVariable);
            }
            curState.curVariable.clear();
            curState.inVariable = true;
            continue;
        }
        if (curState.inVariable) {
            curState.curVariable.push_back(c);
        } else {
            currentArgument.push_back(c);
        }


    }
    if (not curState.curVariable.empty()) {
        currentArgument += getVariableValue(curState.curVariable);
    }
    if (not currentArgument.empty()) {
        res.push_back(currentArgument);
    }

    // Check if the path is wsomething like $ ./<name>.msh
    if (res[0].size() >= 7) {
        if (res[0].substr(0, 2) == "./" && res[0].substr(res[0].size() - 4) == ".msh") {
            std::vector<std::string> newRes;
            newRes.emplace_back("myshell");
            newRes.emplace_back(res[0].substr(2));
            for (int i = 1; i < res.size(); i++) {
                newRes.push_back(res[i]);
            }
            res = newRes;
        }
    }

    return 0;
}



