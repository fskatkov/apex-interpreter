#pragma once

#include "Common/Common.h"
#include "structures/Value/Value.h"

namespace stdlib::SetBuiltins {
    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry);
}
