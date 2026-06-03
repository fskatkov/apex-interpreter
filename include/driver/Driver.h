#pragma once

#include "Common/Common.h"
#include "backend/ExecutionEngine/ExecutionEngine.h"

class Driver {
public:
    explicit Driver() = default;

    static void run();

    static void run(const char* path);
};