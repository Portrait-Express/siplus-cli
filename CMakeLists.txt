cmake_minimum_required(VERSION 3.20)

project(siplus-cli VERSION 1.1.4)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(dependencies)

set(CLI_SOURCES 
    src/main.cpp
)

add_executable(siplus-cli ${CLI_SOURCES})
target_link_libraries(siplus-cli siplus cpptrace::cpptrace)
