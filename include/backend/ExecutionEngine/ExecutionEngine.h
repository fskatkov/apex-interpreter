#pragma once

#include "Common/Common.h"
#include "diagnostics/DiagnosticEngine.h"
#include "backend/BytecodeGenerator/BytecodeGenerator.h"
#include "structures/ExecutionResult/ExecutionResult.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class ExecutionEngine {
public:
    explicit ExecutionEngine();
    ExecutionResult run(std::string& source);
private:
    std::unique_ptr<BytecodeBuffer> buffer;
    const std::uint8_t* address;
    std::vector<std::any> stack;

    ExecutionResult execute();

    std::uint8_t readByte();
    std::any readConstant();

    template<typename T, typename U>
    void executeBinaryOperation(U operation);
    void resetStack();
    void push(const std::any& value);
    std::any pop();
    [[nodiscard]] std::any peek(const int& distance) const;
    [[nodiscard]] bool isNegative(const std::any& value) const;
};
