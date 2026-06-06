#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/AST/Expr/Expression.h"
#include "structures/AST/Stmt/Statement.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, DiagnosticEngine& diagnosticEngine);
    std::vector<std::unique_ptr<Statement>> parse();
private:
    DiagnosticEngine& diagnosticEngine;
    std::vector<Token> tokens;
    std::size_t current;

    std::unique_ptr<Statement> parseDeclarationStatement();
    std::unique_ptr<Statement> parseBlockStatement();
    std::unique_ptr<Statement> parsePrintStatement();
    std::unique_ptr<Statement> parseConditionalStatement();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseExpressionStatement();
    std::unique_ptr<Statement> parseVariableDeclarationStatement(bool isConst);

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignmentExpression();
    std::unique_ptr<Expression> parseBitwiseOrExpression();
    std::unique_ptr<Expression> parseBitwiseXorExpression();
    std::unique_ptr<Expression> parseBitwiseAndExpression();
    std::unique_ptr<Expression> parseEqualityExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseShiftExpression();
    std::unique_ptr<Expression> parseTermExpression();
    std::unique_ptr<Expression> parseFactorExpression();
    std::unique_ptr<Expression> parseExponentialExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePostfixExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();

    bool match(std::initializer_list<TokenKind> kinds);
    bool check(const TokenKind& kind);
    Token advance();
    bool isReachedEnd();
    Token peek();
    Token previous();
    Token consume(const TokenKind& kind, const std::string& message);
    void synchronize();
};
