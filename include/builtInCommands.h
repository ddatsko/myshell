#ifndef MYSHELL_BUILTINCOMMANDS_H
#define MYSHELL_BUILTINCOMMANDS_H

#include <map>
#include <string>
#include <functional>

extern std::map<std::string, std::function<int(int, const char **)>> builtInCommands;

//int mexport(const char **vars);
int mexport(int argc, const char **argv);

int mecho(int argc, const char **argv);

int merrno(int argc, const char **argv);

int mpwd(int argc, const char **argv);

int mcd(int argc, const char **argv);

int mexit(int argc, const char **argv);

int runInThisInterpreter(int argc, const char **argv);


void registerBuildInCommands();


#endif //MYSHELL_BUILTINCOMMANDS_H
