#pragma once

#include "Common/Common.h"

struct LoopContext {
    int continueTarget;
    int loopScopeDepth;
    std::vector<int> breakJumps;
};
