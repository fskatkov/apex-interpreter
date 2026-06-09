#include "structures/BytecodeBuffer/BytecodeBuffer.h"

void BytecodeBuffer::update(const std::uint8_t &byte, const std::size_t &at) {
    code.push_back(byte);
    lines.push_back(at);
}

void BytecodeBuffer::insert(const Value &value, const std::size_t &at) {
    const auto newConstant = add(value);
    update(static_cast<std::uint8_t>(InstructionType::OP_CONSTANT), at);
    update(newConstant, at);
}

std::uint8_t BytecodeBuffer::add(const Value &value) {
    values.emplace_back(value);
    return static_cast<std::uint8_t>(values.size() - 1);
}

std::size_t BytecodeBuffer::at(const std::uint8_t &offset) const {
    return lines[offset];
}
