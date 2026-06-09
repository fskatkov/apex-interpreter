#pragma once

#include "Common/Common.h"
#include "structures/Value/Value.h"
#include "structures/InstructionType/InstructionType.h"

class BytecodeBuffer {
public:
    std::vector<std::uint8_t> code;
    std::vector<Value> values;
    std::vector<std::size_t> lines;

    explicit BytecodeBuffer() = default;

    void update(const std::uint8_t& byte, const std::size_t& at);
    void insert(const Value& value, const std::size_t& at);
    std::uint8_t add(const Value& value);
    [[nodiscard]] std::size_t at(const std::uint8_t& offset) const;
};
