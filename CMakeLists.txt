cmake_minimum_required(VERSION 4.2)
project(apex_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(apex_interpreter main.cpp
        include/engine/Driver/Driver.h
        utils/common/Common.h
        src/engine/Driver/Driver.cpp)

target_include_directories(apex_interpreter PRIVATE utils)
target_include_directories(apex_interpreter PRIVATE include)