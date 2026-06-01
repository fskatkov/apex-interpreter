#pragma once

#include "Common/Common.h"
#include "structures/InstructionType/InstructionType.h"

class BytecodeBuffer {
public:
    std::vector<std::uint8_t> code;
    std::vector<std::any> values;
    std::vector<int> lines;

    explicit BytecodeBuffer() = default;

    void update(const std::uint8_t& byte, const int& at);
    void insert(const std::any& value, const int& at);
    std::uint8_t add(const std::any& value);
    [[nodiscard]] int at(const std::uint8_t& offset) const;
};
