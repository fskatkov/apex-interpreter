#pragma once

#include "Common/Common.h"
#include "frontend/Lexer/Lexer.h"
#include "frontend/Parser/Parser.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class BytecodeGenerator {
public:
    explicit BytecodeGenerator(DiagnosticEngine& diagnosticEngine);
    std::unique_ptr<BytecodeBuffer> generate(std::string& source);
private:
    DiagnosticEngine& diagnosticEngine;
    std::shared_ptr<Lexer> lexer;
    std::shared_ptr<Parser> parser;
    std::vector<Token> tokens;
    std::vector<std::unique_ptr<Statement>> statements;
    std::unique_ptr<Expression> expression;

    void compileStatement(Statement* statement, BytecodeBuffer* buffer);
    void compileExpressionStatement(ExpressionStatement* statement, BytecodeBuffer* buffer);
    void compileVariableStatement(VariableStatement* statement, BytecodeBuffer* buffer);

    void compileExpression(Expression* originalExpression, BytecodeBuffer* buffer);
    void compileVariableExpression(VariableExpression* originalExpression, BytecodeBuffer* buffer);
    void compileGroupingExpression(const GroupingExpression* originalExpression, BytecodeBuffer* buffer);
    void compileBinaryExpression(const BinaryExpression* originalExpression, BytecodeBuffer* buffer);
    void compileUnaryExpression(const UnaryExpression* originalExpression, BytecodeBuffer* buffer);
    void compileLiteralExpression(const LiteralExpression* originalExpression, BytecodeBuffer* buffer);
};
