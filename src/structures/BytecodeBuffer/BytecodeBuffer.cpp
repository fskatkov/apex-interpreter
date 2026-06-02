#include "structures/BytecodeBuffer/BytecodeBuffer.h"

void BytecodeBuffer::update(const std::uint8_t& byte, const std::size_t& at) {
    code.push_back(byte);
    lines.push_back(at);
}

void BytecodeBuffer::insert(const std::any& value, const std::size_t& at) {
    const auto newConstant = add(value);
    update(static_cast<std::uint8_t>(InstructionType::OP_CONSTANT), at);
    update(newConstant, at);
}

std::uint8_t BytecodeBuffer::add(const std::any& value) {
    values.push_back(value);
    return static_cast<std::uint8_t>(values.size() - 1);
}

int BytecodeBuffer::at(const std::uint8_t& offset) const {
    return lines[offset];
}