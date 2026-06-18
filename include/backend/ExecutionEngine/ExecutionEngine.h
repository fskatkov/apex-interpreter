#pragma once

#include "Common/Common.h"
#include "diagnostics/DiagnosticEngine.h"
#include "backend/BytecodeGenerator/BytecodeGenerator.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"
#include "stdlib/StdLib/StdLib.h"

enum class ExecutionResult {
    COMPILETIME_ERROR,
    RUNTIME_ERROR,
    OK,
    HALT,
};

struct Frame {
    std::shared_ptr<Function> function;
    const std::uint8_t* address;
    int stackOffset;
};

using Frames = std::vector<Frame>;

class ExecutionEngine {
public:
    explicit ExecutionEngine(DiagnosticEngine& diagnosticEngine);
    ExecutionResult run(const std::string &input);
private:
    DiagnosticEngine& diagnosticEngine;
    std::string source;
    std::unique_ptr<BytecodeBuffer> buffer = nullptr;
    const std::uint8_t* address = nullptr;

    Frames frames;
    Array stack;

    std::unordered_map<std::string, Value> globalVariables;
    std::unordered_set<std::string> constants;

    StdLib builtins{};

    using Handler = ExecutionResult (ExecutionEngine::*)();
    static const std::array<Handler, 256> dispatchTable;

    ExecutionResult execute();
    std::uint8_t readByte();
    [[maybe_unused]] Value readConstant();

    template<typename... T>
    [[maybe_unused]] ExecutionResult executeOperation(const std::string &symbol, T... items);

    template<std::invocable<std::uint64_t, std::uint64_t> Op>
    double executeBitwiseBinaryOperation(const double& firstNumber, const double& secondNumber, Op operation);

    void resetStack();
    void push(const Value& value);
    [[maybe_unused]] Value pop();
    [[maybe_unused]] [[nodiscard]] Value peek(const int& distance) const;

    [[maybe_unused]] [[nodiscard]] static std::string stringify(const Value& value, bool isNested = false);
    void reportRuntimeError(const std::string& message);

    inline ExecutionResult executeConstant();
    inline ExecutionResult executeTrueLiteral();
    inline ExecutionResult executeFalseLiteral();
    inline ExecutionResult executeNullLiteral();
    inline ExecutionResult executeInterpolatedStringLiteral();

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
    inline ExecutionResult executeBuildSet();
    inline ExecutionResult executeBuildDictionary();
    inline ExecutionResult executeGetIndex();
    inline ExecutionResult executeSetIndex();
    inline ExecutionResult executeGetProperty();

    inline ExecutionResult executeJumpIfFalseOperation();
    inline ExecutionResult executeJumpOperation();
    inline ExecutionResult executePopOperation();
    inline ExecutionResult executeLoop();
    inline ExecutionResult executeDuplicate();
    inline ExecutionResult executeDuplicate2();
    inline ExecutionResult executeFunctionCall();

    inline ExecutionResult executeInOperator();
    inline ExecutionResult executeTypeofOperator();

    inline ExecutionResult executeReturn();
    inline ExecutionResult executeUnknown();
};
