cmake_minimum_required(VERSION 4.2)
project(apex_interpreter)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "main.cpp"
)

add_executable(apex_interpreter ${SOURCES}
        include/stdlib/OSBuiltins/OSBuiltins.h
        src/stdlib/OSBuiltins/OSBuiltins.cpp)

target_include_directories(apex_interpreter PRIVATE utils include)