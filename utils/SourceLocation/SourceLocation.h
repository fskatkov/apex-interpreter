#pragma once

#include <cstddef>

struct SourceLocation {
    std::size_t line = 1;
    std::size_t column = 1;
    std::size_t offset = 0;
    std::size_t length = 1;
};
