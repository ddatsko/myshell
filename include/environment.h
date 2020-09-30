#ifndef MYSHELL_ENVIRONMENT_H
#define MYSHELL_ENVIRONMENT_H


#include <string>

void setPwd();

bool executableExistsInPath(std::string filename);

void exportVariable(std::string varName);

void setVariable(std::string varName, std::string varValue);

std::string getVariableValue(std::string varName);

char **getEnvpForChild();

void initEnvironmentVariables();


#endif
