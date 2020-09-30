cmake_minimum_required(VERSION 3.17)
project(myshell)

set(CMAKE_CXX_STANDARD 17)

find_library(readline REQUIRED)

add_executable(myshell src/main.cpp include/environment.h src/builtInCommands.cpp include/builtInCommands.h src/environment.cpp include/utils.h src/utils.cpp)

target_link_libraries(myshell readline)

include_directories(include)