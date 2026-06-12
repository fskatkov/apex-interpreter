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

struct ControlFlowContext {
    bool isLoop;
    int continueTarget;
    int loopScopeDepth;
    std::vector<int> breakJumps;
    std::vector<int> continueJumps;
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
    std::vector<std::unique_ptr<Stmt>> statements;
    std::unique_ptr<Expr> expression;
    std::vector<Local> locals;
    std::vector<ControlFlowContext> contexts;
    int scopeDepth = 0;

    void compileStatement(Stmt* statement);
    void compileSwitchStatement(const SwitchStatement* statement);
    void compileBreakStatement(const BreakStatement* statement);
    void compileContinueStatement(const ContinueStatement* statement);
    void compileForStatement(const ForStatement* statement);
    void compileWhileStatement(const WhileStatement* statement);
    void compileDoWhileStatement(const DoWhileStatement* statement);
    void compileConditionalStatement(const ConditionalStatement* statement);
    void compileBlockStatement(const BlockStatement* statement);
    void compileVariableStatement(VariableStatement* statement);
    void compileExpressionStatement(const ExpressionStatement* statement);
    void compilePrintStatement(const PrintStatement* statement);

    void compileExpression(Expr* originalExpression);
    void compileAssignmentExpression(const AssignmentExpression* originalExpression);
    void compileTernaryOperatorExpression(const TernaryOperatorExpression* originalExpression);
    void compileLogicalExpression(const LogicalExpression* originalExpression);
    void compileCompoundAssignmentExpression(const CompoundAssignmentExpression* originalExpression);
    void compileUpdateExpression(const UpdateExpression* originalExpression) const;
    void compileVariableExpression(VariableExpression* originalExpression) const;
    void compileGroupingExpression(const GroupingExpression* originalExpression);
    void compileBinaryExpression(const BinaryExpression* originalExpression);
    void compileUnaryExpression(const UnaryExpression* originalExpression);
    void compileLiteralExpression(const LiteralExpression* originalExpression) const;
    void compileInterpolatedStringLiteralExpression(const InterpolatedStringLiteralExpression* originalExpression);
    void compileArrayLiteralExpression(const ArrayLiteralExpression* originalExpression);
    void compileSetLiteralExpression(const SetLiteralExpression* originalExpression);
    void compileDictionaryLiteralExpression(const DictionaryLiteralExpression* statement);
    void compileIndexExpression(const IndexExpression* originalExpression);

    void emitByte(const std::uint8_t& byte, const std::size_t& line) const;
    [[nodiscard]] int emitJump(const std::uint8_t& instruction) const;
    void emitLoop(const int& startingPoint) const;
    void patchJump(const int& offset) const;
    void beginScope();
    void endScope();
    [[nodiscard]] int resolveLocal(const Token& name) const;
};
