#pragma once

#include "Common/Common.h"
#include "structures/Value/Value.h"

namespace stdlib::SetBuiltins {
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> register_methods();
}
