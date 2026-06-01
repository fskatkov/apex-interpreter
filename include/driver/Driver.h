#pragma once

#include "Common/Common.h"
#include "frontend/Lexer/Lexer.h"

constexpr int EXIT_CODE_BROKEN_INPUT = 65;
constexpr int EXIT_CODE_BROKEN_PATH = 66;


class Driver {
public:
    explicit Driver() = default;
    void run();
    void run(const char* path);
private:
    std::string executeFile(const char* path);
    void raiseError(const int& exitCode, const std::string& message);
};