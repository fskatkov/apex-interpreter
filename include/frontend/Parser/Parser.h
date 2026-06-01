#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/AST/Expr/Expression.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens, DiagnosticEngine& diagnosticEngine);
    std::unique_ptr<Expression> parse();
private:
    DiagnosticEngine& diagnosticEngine;

    std::vector<Token> tokens;
    std::size_t current;

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseEqualityExpression();
    std::unique_ptr<Expression> parseComparisonExpression();
    std::unique_ptr<Expression> parseTermExpression();
    std::unique_ptr<Expression> parseFactorExpression();
    std::unique_ptr<Expression> parseUnaryExpression();
    std::unique_ptr<Expression> parsePrimaryExpression();

    bool match(std::initializer_list<TokenKind> kinds);
    bool check(const TokenKind& kind);
    Token advance();
    bool isReachedEnd();
    Token peek();
    Token previous();
    Token consume(const TokenKind& kind, const std::string& message);
    void synchronize();

    struct ParseError : public std::runtime_error {
        explicit ParseError()
            : std::runtime_error("") {  }
    };
};
