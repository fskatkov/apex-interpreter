#pragma once

#include "Common/Common.h"
#include "diagnostics/DiagnosticEngine.h"
#include "backend/ExecutionEngine/ExecutionEngine.h"

struct DriverConfig {
    enum class DriverMode {
        REPL_MODE,
        SCRIPT_MODE
    };

    DriverMode mode;
    std::string path;
};

class Driver {
public:
    int run(const int& argc, const char *argv[]);
private:
    static int execute(const DriverConfig &config);
};