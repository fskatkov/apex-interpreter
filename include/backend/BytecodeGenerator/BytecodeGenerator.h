#pragma once

#include "Common/Common.h"
#include "frontend/Lexer/Lexer.h"
#include "frontend/Parser/Parser.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class BytecodeGenerator {
public:
    std::unique_ptr<BytecodeBuffer> buffer;
    explicit BytecodeGenerator(DiagnosticEngine& diagnosticEngine);
    void generate(std::string& source);
private:
    DiagnosticEngine& diagnosticEngine;

    std::shared_ptr<Lexer> lexer;
    std::shared_ptr<Parser> parser;
    std::vector<Token> tokens;
    std::vector<std::unique_ptr<Statement>> statements;
    std::unique_ptr<Expression> expression;

    void compileStatement(Statement* statement);
    void compilePrintStatement(const PrintStatement* statement);
    void compileExpressionStatement(const ExpressionStatement* statement);
    void compileVariableStatement(VariableStatement* statement);

    void compileExpression(Expression* originalExpression);
    void compileAssignmentExpression(AssignmentExpression* originalExpression);
    void compileVariableExpression(VariableExpression* originalExpression) const;
    void compileGroupingExpression(const GroupingExpression* originalExpression);
    void compileBinaryExpression(const BinaryExpression* originalExpression);
    void compileUnaryExpression(const UnaryExpression* originalExpression);
    void compileLiteralExpression(const LiteralExpression* originalExpression) const;

    void emitByte(const std::uint8_t& byte, const std::size_t& line) const;
};
