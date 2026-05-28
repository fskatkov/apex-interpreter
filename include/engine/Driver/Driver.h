#pragma once

#include "Common/Common.h"

class Driver {
public:
    explicit Driver() = default;
    void run();
    void run(const char* path);
private:
    void executeFile(const char* path);
    void raiseError(const std::string& message);
};