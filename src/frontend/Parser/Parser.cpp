#include "frontend/Parser/Parser.h"

Parser::Parser(const std::vector<Token>& tokens, DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), tokens(tokens), current(0) {  }

std::vector<std::unique_ptr<Statement>> Parser::parse() {
    std::vector<std::unique_ptr<Statement>> statements;
    while (!isReachedEnd()) {
        statements.push_back(parseDeclarationStatement());
    }
    return statements;
}

std::unique_ptr<Statement> Parser::parseDeclarationStatement() {
    if (match({ TokenKind::VAR })) {
        return parseVariableDeclarationStatement();
    }

    return parseStatement();
}

std::unique_ptr<Statement> Parser::parseStatement() {
    return parseExpressionStatement();
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    std::unique_ptr<Expression> expression = parseExpression();
    consume(TokenKind::SEMICOLON, "expect `;` at end of expression");
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

std::unique_ptr<Statement> Parser::parseVariableDeclarationStatement() {
    const auto name = consume(TokenKind::IDENTIFIER, "expect variable name");

    std::unique_ptr<Expression> initializer = nullptr;
    if (match({ TokenKind::EQUALS })) {
        initializer = parseExpression();
    }

    consume(TokenKind::SEMICOLON, "expect `;` at end of variable declaration");
    return std::make_unique<VariableStatement>(name, std::move(initializer));
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseBitwiseOrExpression();
}

std::unique_ptr<Expression> Parser::parseBitwiseOrExpression() {
    std::unique_ptr<Expression> expression = parseBitwiseXorExpression();

    while (match({ TokenKind::BITWISE_OR })) {
        const auto operatorSymbol = previous();
        auto rhs = parseBitwiseXorExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseBitwiseXorExpression() {
    std::unique_ptr<Expression> expression = parseBitwiseAndExpression();

    while (match({ TokenKind::BITWISE_XOR })) {
        const auto operatorSymbol = previous();
        auto rhs = parseBitwiseAndExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseBitwiseAndExpression() {
    std::unique_ptr<Expression> expression = parseEqualityExpression();

    while (match({ TokenKind::BITWISE_AND })) {
        const auto operatorSymbol = previous();
        auto rhs = parseEqualityExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
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
    std::unique_ptr<Expression> expression = parseShiftExpression();

    while (match({ TokenKind::GREATER, TokenKind::GREATER_EQUALS, TokenKind::LESS, TokenKind::LESS_EQUALS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseShiftExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseShiftExpression() {
    std::unique_ptr<Expression> expression = parseTermExpression();

    while (match({ TokenKind::BITWISE_LEFT_SHIFT, TokenKind::BITWISE_RIGHT_SHIFT })) {
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
    std::unique_ptr<Expression> expression = parseExponentialExpression();

    while (match({ TokenKind::MODULO, TokenKind::SLASH, TokenKind::STAR, TokenKind::POWER })) {
        const auto operatorSymbol = previous();
        auto rhs = parseExponentialExpression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseExponentialExpression() {
    std::unique_ptr<Expression> expression = parseUnaryExpression();

    while (match({ TokenKind::POWER })) {
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
    while (match({ TokenKind::BANG, TokenKind::MINUS, TokenKind::BITWISE_NOT })) {
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

    if (match({ TokenKind::IDENTIFIER })) {
        return std::make_unique<VariableExpression>(previous());
    }

    if (match({ TokenKind::LEFT_PAREN })) {
        std::unique_ptr<Expression> expression = parseExpression();
        consume(TokenKind::RIGHT_PAREN, "expect `)` after expression");
        return std::make_unique<GroupingExpression>(std::move(expression));
    }

    diagnosticEngine.report(Diagnostic::DiagnosticKind::Error, peek().sourceLocation, "expected expression");
    return nullptr;
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
    return Token{};
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
