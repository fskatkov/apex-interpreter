#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/AST/Expr/Expr.h"
#include "structures/AST/Stmt/Stmt.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, DiagnosticEngine& diagnosticEngine);
    std::vector<std::unique_ptr<Stmt>> parse();
private:
    DiagnosticEngine& diagnosticEngine;
    std::vector<Token> tokens;
    std::size_t current;

    std::unique_ptr<Stmt> parseDeclarationStatement();
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseSwitchStatement();
    std::unique_ptr<Stmt> parseBreakStatement();
    std::unique_ptr<Stmt> parseContinueStatement();
    std::unique_ptr<Stmt> parseForStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseDoWhileStatement();
    std::unique_ptr<Stmt> parseConditionalStatement();
    std::unique_ptr<Stmt> parseBlockStatement();
    std::unique_ptr<Stmt> parsePrintStatement();
    std::unique_ptr<Stmt> parseExpressionStatement();
    std::unique_ptr<Stmt> parseVariableDeclarationStatement(bool isConst);
    std::unique_ptr<Stmt> parseFunctionDeclarationStatement();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignmentExpression();
    std::unique_ptr<Expr> parseTernaryOperatorExpression();
    std::unique_ptr<Expr> parseLogicalOrExpression();
    std::unique_ptr<Expr> parseLogicalAndExpression();
    std::unique_ptr<Expr> parseBitwiseOrExpression();
    std::unique_ptr<Expr> parseBitwiseXorExpression();
    std::unique_ptr<Expr> parseBitwiseAndExpression();
    std::unique_ptr<Expr> parseEqualityExpression();
    std::unique_ptr<Expr> parseComparisonExpression();
    std::unique_ptr<Expr> parseShiftExpression();
    std::unique_ptr<Expr> parseTermExpression();
    std::unique_ptr<Expr> parseFactorExpression();
    std::unique_ptr<Expr> parseExponentialExpression();
    std::unique_ptr<Expr> parseUnaryExpression();
    std::unique_ptr<Expr> parsePostfixExpression();
    std::unique_ptr<Expr> parseFunctionCallExpression();
    std::unique_ptr<Expr> parsePrimaryExpression();

    bool match(std::initializer_list<TokenKind> kinds);
    bool check(const TokenKind& kind);
    Token advance();
    bool isReachedEnd();
    Token peek();
    Token previous();
    Token consume(const TokenKind& kind, const std::string& message);
    void synchronize();
};
