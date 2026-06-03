#pragma once

#include "Common/Common.h"
#include "backend/ExecutionEngine/ExecutionEngine.h"

class REPL {
public:
    explicit REPL() = default;
    static void run();
    static void run(const char* path);
private:
    static bool handleSpecialCommands(const std::string& command);
    static bool isIncompleteCommand(const std::string& buffer);
};