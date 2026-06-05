#pragma once

#include "Common/Common.h"
#include "diagnostics/DiagnosticEngine.h"
#include "backend/BytecodeGenerator/BytecodeGenerator.h"
#include "structures/ExecutionResult/ExecutionResult.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class ExecutionEngine {
public:
    explicit ExecutionEngine(std::string& source, DiagnosticEngine& diagnosticEngine);
    ExecutionResult run();
private:
    DiagnosticEngine& diagnosticEngine;
    std::string source;

    std::unordered_map<std::string, std::any> globalVariables;
    std::unordered_set<std::string> constants;
    std::unique_ptr<BytecodeBuffer> buffer;
    const std::uint8_t* address;
    std::vector<std::any> stack;

    ExecutionResult execute();

    std::uint8_t readByte();
    std::any readConstant();

    template<typename T, typename U>
    void executeBinaryOperation(U operation);
    template<typename T>
    double executeBitwiseBinaryOperation(const double& firstNumber, const double& secondNumber, T operation);
    void resetStack();
    void push(const std::any& value);
    std::any pop();
    [[nodiscard]] std::any peek(const int& distance) const;
    void reportRuntimeError(const std::string& message);
};
