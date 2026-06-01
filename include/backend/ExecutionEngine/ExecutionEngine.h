#pragma once

#include "Common/Common.h"
#include "structures/ExecutionResult/ExecutionResult.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class ExecutionEngine {
public:
    explicit ExecutionEngine();
    ExecutionResult run(const std::string& source);
private:
    std::shared_ptr<BytecodeBuffer> buffer;
    const std::uint8_t* address;
    std::vector<std::any> stack;

    ExecutionResult execute();

    template<typename T, typename U>
    void executeBinaryOperation(U operation);
    void resetStack();
    void push(const std::any& value);
    std::any pop();
    [[nodiscard]] std::any peek(const int& distance) const;
    [[nodiscard]] bool isNegative(const std::any& value) const;
};
