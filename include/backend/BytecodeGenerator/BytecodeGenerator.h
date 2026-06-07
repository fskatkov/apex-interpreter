#pragma once

#include "Common/Common.h"
#include "frontend/Lexer/Lexer.h"
#include "frontend/Parser/Parser.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

struct Local {
    Token name;
    int depth = 0;
};

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
    std::vector<Local> locals;
    int scopeDepth = 0;

    void compileStatement(Statement* statement);
    void compileWhileStatement(const WhileStatement* statement);
    void compileConditionalStatement(const ConditionalStatement* statement);
    void compileBlockStatement(const BlockStatement* statement);
    void compileVariableStatement(VariableStatement* statement);
    void compileExpressionStatement(const ExpressionStatement* statement);
    void compilePrintStatement(const PrintStatement* statement);

    void compileExpression(Expression* originalExpression);
    void compileAssignmentExpression(AssignmentExpression* originalExpression);
    void compileLogicalExpression(const LogicalExpression* originalExpression);
    void compileCompoundAssignmentExpression(const CompoundAssignmentExpression* originalExpression);
    void compileUpdateExpression(const UpdateExpression* originalExpression) const;
    void compileVariableExpression(VariableExpression* originalExpression) const;
    void compileGroupingExpression(const GroupingExpression* originalExpression);
    void compileBinaryExpression(const BinaryExpression* originalExpression);
    void compileUnaryExpression(const UnaryExpression* originalExpression);
    void compileLiteralExpression(const LiteralExpression* originalExpression) const;

    void emitByte(const std::uint8_t& byte, const std::size_t& line) const;
    [[nodiscard]] int emitJump(const std::uint8_t& instruction) const;
    void emitLoop(const int& startingPoint);
    void patchJump(const int& offset) const;
    void beginScope();
    void endScope();
    [[nodiscard]] int resolveLocal(const Token& name) const;
};
