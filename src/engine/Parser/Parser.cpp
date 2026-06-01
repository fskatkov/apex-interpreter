#include "engine/Parser/Parser.h"

#include "structures/AST/BinaryExpression.h"

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {  }

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseEqualityExpression();
}

std::unique_ptr<Expression> Parser::parseEqualityExpression() {
    std::unique_ptr<Expression> expression = parseComparisonExpression();

    while (match({TokenKind::BANG_EQUALS, TokenKind::EQUALS_EQUALS})) {
        const auto operatorSymbol = previous();
        std::unique_ptr<Expression> rhs = parseComparisonExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseComparisonExpression() {

}

bool Parser::match(std::initializer_list<TokenKind> kinds) {
    for (const auto& kind : kinds) {
        if (check(kind)) {
            advance();
            return true;
        }
    }

    return false;
}

bool Parser::check(const TokenKind& kind) {
    if (isReachedEnd()) {
        return false;
    }

    return peek().kind == kind;
}

Token Parser::advance() {
    if (!isReachedEnd()) {
        current++;
    }

    return previous();
}

bool Parser::isReachedEnd() {
    return peek().kind == TokenKind::END_OF_FILE;
}

Token Parser::peek() {
    return tokens[current];
}

Token Parser::previous() {
    return tokens[current - 1];
}
