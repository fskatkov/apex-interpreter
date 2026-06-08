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
        return parseVariableDeclarationStatement(false);
    }

    if (match({ TokenKind::CONST })) {
        return parseVariableDeclarationStatement(true);
    }

    return parseStatement();
}

std::unique_ptr<Statement> Parser::parseStatement() {
    if (match({ TokenKind::SWITCH })) {
        return parseSwitchStatement();
    }

    if (match({ TokenKind::BREAK })) {
        return parseBreakStatement();
    }

    if (match({ TokenKind::CONTINUE })) {
        return parseContinueStatement();
    }

    if (match({ TokenKind::FOR })) {
        return parseForStatement();
    }

    if (match({ TokenKind::WHILE })) {
        return parseWhileStatement();
    }

    if (match({ TokenKind::DO })) {
        return parseDoWhileStatement();
    }

    if (match({ TokenKind::IF })) {
        return parseConditionalStatement();
    }

    if (match({ TokenKind::LEFT_BRACE })) {
        return parseBlockStatement();
    }

    if (match({ TokenKind::PRINT })) {
        return parsePrintStatement();
    }

    return parseExpressionStatement();
}

std::unique_ptr<Statement> Parser::parseSwitchStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");

    auto condition = parseExpression();

    consume(TokenKind::RIGHT_PAREN, "expected `(` in expression list");
    consume(TokenKind::LEFT_BRACE, "expected `{` before `switch` body");

    std::vector<std::unique_ptr<CaseStatement>> cases;
    while (match({ TokenKind::CASE })) {
        auto caseExpression = parseExpression();

        consume(TokenKind::COLON, "expected `:` at end of `case` condition");

        auto caseBody = parseStatement();

        auto caseStatement = std::make_unique<CaseStatement>(
            std::move(caseExpression),
            std::move(caseBody)
        );
        cases.push_back(std::move(caseStatement));
    }

    consume(TokenKind::DEFAULT, "expected `default` case in `switch` body");
    consume(TokenKind::COLON, "expected `:` at end of `default` clause");

    auto defaultCase = parseStatement();

    consume(TokenKind::RIGHT_BRACE, "expected `}` at end of `switch` body");

    return std::make_unique<SwitchStatement>(
        std::move(condition),
        std::move(cases),
        std::move(defaultCase)
    );
}

std::unique_ptr<Statement> Parser::parseBreakStatement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `break` statement");
    return std::make_unique<BreakStatement>(keyword);
}

std::unique_ptr<Statement> Parser::parseContinueStatement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `continue` statement");
    return std::make_unique<ContinueStatement>(keyword);
}

std::unique_ptr<Statement> Parser::parseForStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");

    std::unique_ptr<Statement> initializer = nullptr;
    if (match({ TokenKind::VAR })) {
        initializer = parseVariableDeclarationStatement(false);
    } else {
        initializer = parseExpressionStatement();
    }

    std::unique_ptr<Expression> condition = nullptr;
    if (!check({ TokenKind::SEMICOLON })) {
        condition = parseExpression();
    }

    consume(TokenKind::SEMICOLON, "expected `;` at end of `for` condition");

    std::unique_ptr<Expression> increment = nullptr;
    if (!check({ TokenKind::RIGHT_PAREN })) {
        increment = parseExpression();
    }

    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    std::unique_ptr<Statement> body = parseStatement();
    return std::make_unique<ForStatement>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto body = parseStatement();
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::parseDoWhileStatement() {
    auto body = parseStatement();

    consume(TokenKind::WHILE, "expected `while` statement after `do` body");
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");

    auto condition = parseExpression();

    consume(TokenKind::RIGHT_PAREN, "expected `(` in expression list");
    consume(TokenKind::SEMICOLON, "expected `;` at end of `do-while`");

    return std::make_unique<DoWhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Statement> Parser::parseConditionalStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto thenBranch = parseStatement();
    std::unique_ptr<Statement> elseBranch = nullptr;
    if (match({ TokenKind::ELSE })) {
        elseBranch = parseStatement();
    }

    return std::make_unique<ConditionalStatement>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

std::unique_ptr<Statement> Parser::parseBlockStatement() {
    auto parseStatements = [this]() {
        std::vector<std::unique_ptr<Statement>> statements;

        while (!check(TokenKind::RIGHT_BRACE) && !isReachedEnd()) {
            statements.push_back(parseDeclarationStatement());
        }

        consume(TokenKind::RIGHT_BRACE, "expected `}` at end of block statement");
        return statements;
    };

    return std::make_unique<BlockStatement>(std::move(parseStatements()));
}

std::unique_ptr<Statement> Parser::parsePrintStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` before print expression");
    auto expression = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` at end of print statement");
    consume(TokenKind::SEMICOLON, "expected `;` at end of print statement");
    return std::make_unique<PrintStatement>(std::move(expression));
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto expression = parseExpression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of expression");
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

std::unique_ptr<Statement> Parser::parseVariableDeclarationStatement(bool isConst) {
    const auto name = consume(TokenKind::IDENTIFIER, "expected variable name");

    std::unique_ptr<Expression> initializer = nullptr;
    if (match({ TokenKind::EQUALS })) {
        initializer = parseExpression();
    } else if (isConst) {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            name.sourceLocation,
            "const variables must be initialized"
        );
        return nullptr;
    }

    consume(TokenKind::SEMICOLON, "expected `;` at end of variable declaration");
    return std::make_unique<VariableStatement>(name, std::move(initializer), isConst);
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::unique_ptr<Expression> Parser::parseAssignmentExpression() {
    auto expression = parseLogicalOrExpression();

    if (match({ TokenKind::EQUALS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseAssignmentExpression();

        if (const auto* variableExpression = dynamic_cast<VariableExpression*>(expression.get())) {
            const auto name = variableExpression->name;
            return std::make_unique<AssignmentExpression>(name, std::move(rhs));
        }
    }

    const std::initializer_list<TokenKind> compoundOperators{
        TokenKind::PLUS_EQUALS, TokenKind::MINUS_EQUALS, TokenKind::STAR_EQUALS, TokenKind::SLASH_EQUALS,
        TokenKind::MODULO_EQUALS, TokenKind::BITWISE_AND_EQUALS, TokenKind::BITWISE_OR_EQUALS, TokenKind::BITWISE_XOR_EQUALS,
        TokenKind::BITWISE_LEFT_SHIFT_EQUALS, TokenKind::BITWISE_RIGHT_SHIFT_EQUALS
    };

    if (match(compoundOperators)) {
        const auto operatorSymbol = previous();
        auto rhs = parseLogicalOrExpression();
        return std::make_unique<CompoundAssignmentExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseLogicalOrExpression() {
    auto expression = parseLogicalAndExpression();

    while (match({ TokenKind::OR })) {
        const auto operatorSymbol = previous();
        auto rhs = parseLogicalAndExpression();
        expression = std::make_unique<LogicalExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseLogicalAndExpression() {
    auto expression = parseBitwiseOrExpression();

    while (match({ TokenKind::AND })) {
        const auto operatorSymbol = previous();
        auto rhs = parseBitwiseOrExpression();
        expression = std::make_unique<LogicalExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expression> Parser::parseBitwiseOrExpression() {
    auto expression = parseBitwiseXorExpression();

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
    auto expression = parseBitwiseAndExpression();

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
    auto expression = parseEqualityExpression();

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
    auto expression = parseComparisonExpression();

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
    auto expression = parseShiftExpression();

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
    auto expression = parseTermExpression();

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
    auto expression = parseFactorExpression();

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
    auto expression = parseExponentialExpression();

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
    auto expression = parseUnaryExpression();

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

    return parsePostfixExpression();
}

std::unique_ptr<Expression> Parser::parsePostfixExpression() {
    auto expression = parsePrimaryExpression();

    if (match({ TokenKind::INCREMENT, TokenKind::DECREMENT })) {
        const auto operatorSymbol = previous();
        return std::make_unique<UpdateExpression>(
            operatorSymbol,
            std::move(expression)
        );
    }

    return expression;
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
        consume(TokenKind::RIGHT_PAREN, "expected `)` after expression");
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
