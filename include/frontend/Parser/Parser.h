#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/AST/Expr/Expr.h"
#include "structures/AST/Stmt/Stmt.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, DiagnosticEngine& diagnosticEngine);
    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> parse();
private:
    DiagnosticEngine& diagnosticEngine;
    std::vector<Token> tokens;
    std::size_t current{0};

    [[nodiscard]] std::unique_ptr<Stmt> parseDeclarationStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseFunctionDeclarationStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseBlockStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseVariableDeclarationStatement(bool isConst);
    [[nodiscard]] std::unique_ptr<Stmt> parseSwitchStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseForStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseWhileStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseDoWhileStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseConditionalStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseExpressionStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseBreakStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseContinueStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parseReturnStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parsePrintStatement();

    [[nodiscard]] std::unique_ptr<Expr> parseExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseAssignmentExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseTernaryOperatorExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseLogicalOrExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseLogicalAndExpression();
    [[nodiscard]] std::unique_ptr<Expr> parsePipeOperation();
    [[nodiscard]] std::unique_ptr<Expr> parseCaretOperation();
    [[nodiscard]] std::unique_ptr<Expr> parseAmpersandOperation();
    [[nodiscard]] std::unique_ptr<Expr> parseEqualityExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseComparisonExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseAngleOperation();
    [[nodiscard]] std::unique_ptr<Expr> parseTermExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseFactorExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseExponentialExpression();
    [[nodiscard]] std::unique_ptr<Expr> parseUnaryExpression();
    [[nodiscard]] std::unique_ptr<Expr> parsePostfixExpression();
    [[nodiscard]] std::unique_ptr<Expr> parsePrimaryExpression();

    [[nodiscard]] bool match(std::initializer_list<TokenKind> kinds);
    [[nodiscard]] bool check(const TokenKind& kind);
    Token advance();
    [[nodiscard]] bool isReachedEnd();
    [[nodiscard]] Token peek();
    [[nodiscard]] Token previous();
    Token consume(const TokenKind& kind, const std::string& message);

    void synchronize();
};
