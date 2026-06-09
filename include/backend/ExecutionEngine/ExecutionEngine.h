#pragma once

#include "Common/Common.h"
#include "diagnostics/DiagnosticEngine.h"
#include "backend/BytecodeGenerator/BytecodeGenerator.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

enum class ExecutionResult {
    COMPILETIME_ERROR,
    RUNTIME_ERROR,
    OK,
    HALT,
};

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

    using Handler = ExecutionResult (ExecutionEngine::*)();
    static const std::array<Handler, 256> dispatchTable;

    ExecutionResult execute();

    inline ExecutionResult executeConstant();
    inline ExecutionResult executeTrueLiteral();
    inline ExecutionResult executeFalseLiteral();
    inline ExecutionResult executeNullLiteral();

    inline ExecutionResult executeAddition();
    inline ExecutionResult executeSubtraction();
    inline ExecutionResult executeMultiplication();
    inline ExecutionResult executeDivision();
    inline ExecutionResult executeModuloDivision();
    inline ExecutionResult executePower();
    inline ExecutionResult executeNotOperation();
    inline ExecutionResult executeNegation();
    inline ExecutionResult executeBitwiseAnd();
    inline ExecutionResult executeBitwiseOr();
    inline ExecutionResult executeBitwiseXor();
    inline ExecutionResult executeBitwiseNot();
    inline ExecutionResult executeBitwiseLeftShift();
    inline ExecutionResult executeBitwiseRightShift();

    inline ExecutionResult executeEquality();
    inline ExecutionResult executeGreaterOperation();
    inline ExecutionResult executeGreaterThanOperation();
    inline ExecutionResult executeLessOperation();
    inline ExecutionResult executeLessThanOperation();

    inline ExecutionResult executeDefineGlobalVariable();
    inline ExecutionResult executeDefineConstantVariable();
    inline ExecutionResult executeGetGlobalVariable();
    inline ExecutionResult executeSetGlobalVariable();
    inline ExecutionResult executeGetLocalVariable();
    inline ExecutionResult executeSetLocalVariable();
    inline ExecutionResult executeBuildArray();
    inline ExecutionResult executeGetIndex();
    inline ExecutionResult executeSetIndex();

    inline ExecutionResult executeJumpIfFalseOperation();
    inline ExecutionResult executeJumpOperation();
    inline ExecutionResult executePopOperation();
    inline ExecutionResult executeLoop();
    inline ExecutionResult executeDuplicate();

    inline ExecutionResult executePrint();
    inline ExecutionResult executeReturn();
    inline ExecutionResult executeUnknown();

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
