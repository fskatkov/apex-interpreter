#include "frontend/Parser/Parser.h"

Parser::Parser(const std::vector<Token>& tokens, DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), tokens(tokens), current(0) {  }

std::unique_ptr<Expression> Parser::parse() {
    try {
        return parseExpression();
    } catch (ParseError& error) {
        return nullptr;
    }
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseEqualityExpression();
}

std::unique_ptr<Expression> Parser::parseEqualityExpression() {
    std::unique_ptr<Expression> expression = parseComparisonExpression();

    while (match({TokenKind::BANG_EQUALS, TokenKind::EQUALS_EQUALS})) {
        const auto operatorSymbol = previous();
        auto rhs = parseComparisonExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseComparisonExpression() {
    std::unique_ptr<Expression> expression = parseTermExpression();

    while (match({ TokenKind::GREATER, TokenKind::GREATER_EQUALS, TokenKind::LESS, TokenKind::LESS_EQUALS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseTermExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseTermExpression() {
    std::unique_ptr<Expression> expression = parseFactorExpression();

    while (match({ TokenKind::PLUS, TokenKind::MINUS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseFactorExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseFactorExpression() {
    std::unique_ptr<Expression> expression = parseUnaryExpression();

    while (match({ TokenKind::MODULO, TokenKind::SLASH, TokenKind::STAR, TokenKind::POWER })) {
        const auto operatorSymbol = previous();
        auto rhs = parseUnaryExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseUnaryExpression() {
    while (match({ TokenKind::BANG, TokenKind::MINUS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseUnaryExpression();
        return std::make_unique<UnaryExpression>(
            operatorSymbol,
            std::move(rhs)
        );
    }

    return parsePrimaryExpression();
}

std::unique_ptr<Expression> Parser::parsePrimaryExpression() {
    if (match({ TokenKind::TRUE })) {
        return std::make_unique<LiteralExpression>(true);
    }

    if (match({ TokenKind::FALSE })) {
        return std::make_unique<LiteralExpression>(false);
    }

    if (match({ TokenKind::NIL })) {
        return std::make_unique<LiteralExpression>(NULL);
    }

    if (match({ TokenKind::NUMBER, TokenKind::STRING, TokenKind::CHARACTER })) {
        return std::make_unique<LiteralExpression>(previous().literal);
    }

    if (match({ TokenKind::LEFT_PAREN })) {
        std::unique_ptr<Expression> expression = parseExpression();
        consume(TokenKind::RIGHT_PAREN, "expect `)` after expression");
        return std::make_unique<GroupingExpression>(std::move(expression));
    }

    diagnosticEngine.report(Diagnostic::DiagnosticKind::Error, peek().sourceLocation, "expected expression");
    throw ParseError();
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

Token Parser::consume(const TokenKind& kind, const std::string& message) {
    if (check(kind)) {
        return advance();
    }

    diagnosticEngine.report(Diagnostic::DiagnosticKind::Error, peek().sourceLocation, message);
    throw ParseError();
}

void Parser::synchronize() {
    advance();

    while (!isReachedEnd()) {
        if (previous().kind == TokenKind::SEMICOLON) {
            return;
        }

        switch (peek().kind) {
            case TokenKind::CLASS:
            case TokenKind::FUNCTION:
            case TokenKind::VAR:
            case TokenKind::FOR:
            case TokenKind::IF:
            case TokenKind::WHILE:
            case TokenKind::RETURN:
                return;
            default:
                advance();
        }
    }
}
