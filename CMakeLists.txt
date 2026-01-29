cmake_minimum_required(VERSION 3.20)

option(WITH_STATIC_CRT "(MSVC Visual C++) Enable to statically link CRT, which avoids requiring users to install the redistribution package.
 To disable with: -DWITH_STATIC_CRT=Off" On)

if(WITH_STATIC_CRT)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif(WITH_STATIC_CRT)

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
