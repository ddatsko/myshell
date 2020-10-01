cmake_minimum_required(VERSION 3.17)
project(myshell CXX)

set(CMAKE_CXX_STANDARD 17)

# Options
option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors." ON)
option(ENABLE_SANITIZERS "Use sanitizers to detect errors." OFF)
option(ENABLE_CONAN "Use Conan as a package manager." OFF)

# Optimize if anything else is not specified
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

# Find external libraries
find_package(Boost 1.71.0 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
find_library(readline readline REQUIRED)


# MYSHELL
add_executable(myshell src/myshell/main.cpp include/myshell/environment.h src/myshell/builtInCommands.cpp include/myshell/builtInCommands.h src/myshell/environment.cpp include/myshell/utils.h src/myshell/utils.cpp include/myshell/commandsRunning.h src/myshell/commandsRunning.cpp)
target_link_libraries(myshell readline)

# MYCAT
add_executable(mycat src/mycat/main.cpp src/mycat/programOptionsParse.cpp src/mycat/io.cpp include/mycat/io/io.h include/mycat/programOptions/programOptionsParse.h)
target_link_libraries(mycat Boost::program_options)

include_directories(include)


# Sanitizers configuration
if (ENABLE_SANITIZERS)
    set(SANITIZE_UNDEFINED ON)
    # Only one of Memory, Address, or Thread sanitizers is applicable at the time
    set(SANITIZE_MEMORY ON)
    #    set(SANITIZE_ADDRESS ON)
    #    set(SANITIZE_THREAD ON)
    find_package(Sanitizers)

    # CMake targets to which apply sanitizers
    add_sanitizers(myshell)
endif ()
