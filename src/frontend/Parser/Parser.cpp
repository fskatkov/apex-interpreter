#include "frontend/Parser/Parser.h"

Parser::Parser(std::vector<Token> tokens, DiagnosticEngine &diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), tokens(std::move(tokens)) {
}

std::vector<std::unique_ptr<Stmt> > Parser::parse() {
    std::vector<std::unique_ptr<Stmt> > statements;
    while (!isReachedEnd()) {
        statements.push_back(parseDeclarationStatement());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::parseDeclarationStatement() {
    if (match({ TokenKind::FUNCTION })) return parseFunctionDeclarationStatement();
    if (match({TokenKind::VAR})) return parseVariableDeclarationStatement(false);
    if (match({TokenKind::CONST})) return parseVariableDeclarationStatement(true);
    return parseStatement();
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (match({TokenKind::SWITCH})) return parseSwitchStatement();
    if (match({TokenKind::BREAK})) return parseBreakStatement();
    if (match({TokenKind::CONTINUE})) return parseContinueStatement();
    if (match({TokenKind::FOR})) return parseForStatement();
    if (match({TokenKind::WHILE})) return parseWhileStatement();
    if (match({TokenKind::DO})) return parseDoWhileStatement();
    if (match({TokenKind::IF})) return parseConditionalStatement();
    if (match({TokenKind::LEFT_BRACE})) return parseBlockStatement();
    if (match({TokenKind::RETURN})) return parseReturnStatement();
    return parseExpressionStatement();
}

std::unique_ptr<Stmt> Parser::parseFunctionDeclarationStatement() {
    auto name = consume(TokenKind::IDENTIFIER, "expected function name");
    consume(TokenKind::LEFT_PAREN, "expected `(` before the argument list");

    std::vector<Token> arguments;
    if (!check({ TokenKind::RIGHT_PAREN })) {
        do {
            if (arguments.size() >= 255) [[unlikely]] {
                diagnosticEngine.report(
                    Diagnostic::DiagnosticKind::Error,
                    arguments.back().sourceLocation,
                    "argument list cannot exceed 255 arguments"
                );

                return nullptr;
            }

            arguments.push_back(consume(TokenKind::IDENTIFIER, "expected argument name"));
        } while (match({ TokenKind::COMMA }));
    }

    consume(TokenKind::RIGHT_PAREN, "expected `)` at end of argument list");
    consume(TokenKind::LEFT_BRACE, "expected `{` before function body");

    auto body = parseBlockStatement();
    return std::make_unique<FunctionStatement>(
        name,
        arguments,
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::parseBlockStatement() {
    std::vector<std::unique_ptr<Stmt> > statements;
    while (!check(TokenKind::RIGHT_BRACE) && !isReachedEnd()) {
        statements.push_back(parseDeclarationStatement());
    }

    consume(TokenKind::RIGHT_BRACE, "expected `}` at end of block statement");
    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseVariableDeclarationStatement(bool isConst) {
    const auto name = consume(TokenKind::IDENTIFIER, "expected variable name");

    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenKind::EQUALS})) {
        initializer = parseExpression();
    } else if (isConst) [[unlikely]] {
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

std::unique_ptr<Stmt> Parser::parseSwitchStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");
    consume(TokenKind::LEFT_BRACE, "expected `{` before `switch` body");

    std::vector<std::unique_ptr<CaseStatement> > cases;
    while (match({TokenKind::CASE})) {
        auto caseExpression = parseExpression();
        consume(TokenKind::COLON, "expected `:` at end of `case` condition");
        auto caseBody = parseStatement();

        cases.push_back(std::make_unique<CaseStatement>(
            std::move(caseExpression),
            std::move(caseBody)
        ));
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

std::unique_ptr<Stmt> Parser::parseForStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");

    auto initializer = match({ TokenKind::VAR })
        ? parseVariableDeclarationStatement(false)
        : parseExpressionStatement();

    auto condition = check({TokenKind::SEMICOLON}) ? nullptr : parseExpression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `for` condition");

    auto increment = check({TokenKind::RIGHT_PAREN}) ? nullptr : parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto body = parseStatement();

    return std::make_unique<ForStatement>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto body = parseStatement();
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseDoWhileStatement() {
    auto body = parseStatement();

    consume(TokenKind::WHILE, "expected `while` statement after `do` body");
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `(` in expression list");
    consume(TokenKind::SEMICOLON, "expected `;` at end of `do-while`");

    return std::make_unique<DoWhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parseConditionalStatement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parseExpression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto thenBranch = parseStatement();
    auto elseBranch = !match({ TokenKind::ELSE }) ? nullptr : parseStatement();

    return std::make_unique<ConditionalStatement>(
        std::move(condition),
        std::move(thenBranch),
        std::move(elseBranch)
    );
}

std::unique_ptr<Stmt> Parser::parseExpressionStatement() {
    auto expression = parseExpression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of expression");
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

std::unique_ptr<Stmt> Parser::parseBreakStatement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `break` statement");
    return std::make_unique<BreakStatement>(keyword);
}

std::unique_ptr<Stmt> Parser::parseContinueStatement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `continue` statement");
    return std::make_unique<ContinueStatement>(keyword);
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    auto keyword = previous();
    auto value = check({ TokenKind::SEMICOLON }) ? nullptr : parseExpression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `return` statement");
    return std::make_unique<ReturnStatement>(std::move(keyword), std::move(value));
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignmentExpression();
}

std::unique_ptr<Expr> Parser::parseAssignmentExpression() {
    auto expression = parseTernaryOperatorExpression();

    if (match({ TokenKind::EQUALS })) {
        const auto operatorSymbol = previous();
        auto rhs = parseAssignmentExpression();

        if (dynamic_cast<VariableExpression *>(expression.get()) ||
            dynamic_cast<IndexExpression *>(expression.get())) [[likely]] {

            return std::make_unique<AssignmentExpression>(
                std::move(expression),
                operatorSymbol,
                std::move(rhs)
            );
        }

        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            operatorSymbol.sourceLocation,
            "invalid assignment target"
        );
    }

    if (match({
        TokenKind::PLUS_EQUALS, TokenKind::MINUS_EQUALS, TokenKind::STAR_EQUALS, TokenKind::SLASH_EQUALS,
        TokenKind::MODULO_EQUALS, TokenKind::AMPERSAND_EQUALS, TokenKind::PIPE_EQUALS,
        TokenKind::CARET_EQUALS, TokenKind::LEFT_ANGLE_EQUALS, TokenKind::RIGHT_ANGLE_EQUALS
    })) {
        const auto operatorSymbol = previous();
        auto rhs = parseTernaryOperatorExpression();

        if (dynamic_cast<VariableExpression *>(expression.get()) ||
            dynamic_cast<IndexExpression *>(expression.get())) [[likely]] {

            return std::make_unique<CompoundAssignmentExpression>(
                std::move(expression),
                operatorSymbol,
                std::move(rhs)
            );
        }

        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            operatorSymbol.sourceLocation,
            "invalid assignment target"
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseTernaryOperatorExpression() {
    auto expression = parseLogicalOrExpression();

    if (match({ TokenKind::QUESTION_MARK })) {
        auto thenBranch = parseTernaryOperatorExpression();
        consume(TokenKind::COLON, "expected `:` after `? ...` in ternary expression");
        auto elseBranch = parseTernaryOperatorExpression();
        expression = std::make_unique<TernaryOperatorExpression>(
            std::move(expression),
            std::move(thenBranch),
            std::move(elseBranch)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseLogicalOrExpression() {
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

std::unique_ptr<Expr> Parser::parseLogicalAndExpression() {
    auto expression = parsePipeOperation();

    while (match({ TokenKind::AND })) {
        const auto operatorSymbol = previous();
        auto rhs = parsePipeOperation();
        expression = std::make_unique<LogicalExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parsePipeOperation() {
    auto expression = parseCaretOperation();

    while (match({ TokenKind::PIPE })) {
        const auto operatorSymbol = previous();
        auto rhs = parseCaretOperation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseCaretOperation() {
    auto expression = parseAmpersandOperation();

    while (match({ TokenKind::CARET })) {
        const auto operatorSymbol = previous();
        auto rhs = parseAmpersandOperation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseAmpersandOperation() {
    auto expression = parseEqualityExpression();

    while (match({ TokenKind::AMPERSAND })) {
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

std::unique_ptr<Expr> Parser::parseEqualityExpression() {
    auto expression = parseComparisonExpression();

    while (match({ TokenKind::BANG_EQUALS, TokenKind::EQUALS_EQUALS })) {
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

std::unique_ptr<Expr> Parser::parseComparisonExpression() {
    auto expression = parseAngleOperation();

    while (match({
        TokenKind::GREATER, TokenKind::GREATER_EQUALS, TokenKind::LESS, TokenKind::LESS_EQUALS, TokenKind::IN
    })) {
        const auto operatorSymbol = previous();
        auto rhs = parseAngleOperation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operatorSymbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parseAngleOperation() {
    auto expression = parseTermExpression();

    while (match({ TokenKind::LEFT_ANGLE, TokenKind::RIGHT_ANGLE })) {
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

std::unique_ptr<Expr> Parser::parseTermExpression() {
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

std::unique_ptr<Expr> Parser::parseFactorExpression() {
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

std::unique_ptr<Expr> Parser::parseExponentialExpression() {
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

std::unique_ptr<Expr> Parser::parseUnaryExpression() {
    while (match({ TokenKind::BANG, TokenKind::MINUS, TokenKind::TILDE, TokenKind::TYPEOF })) {
        const auto operatorSymbol = previous();
        auto rhs = parseUnaryExpression();
        return std::make_unique<UnaryExpression>(
            operatorSymbol,
            std::move(rhs)
        );
    }

    return parsePostfixExpression();
}

std::unique_ptr<Expr> Parser::parsePostfixExpression() {
    auto expression = parsePrimaryExpression();

    while (true) {
        if (match({ TokenKind::LEFT_BRACKET })) {
            const auto bracket = previous();
            auto index = parseExpression();
            consume(TokenKind::RIGHT_BRACKET, "expected `]` after array index");

            expression = std::make_unique<IndexExpression>(
                std::move(expression),
                bracket,
                std::move(index)
            );
        } else if (match({ TokenKind::LEFT_PAREN })) {
            std::vector<std::unique_ptr<Expr>> arguments;

            if (!check({ TokenKind::RIGHT_PAREN })) {
                do {
                    if (arguments.size() >= 255) [[unlikely]] {
                        diagnosticEngine.report(
                            Diagnostic::DiagnosticKind::Error,
                            peek().sourceLocation,
                            "function call cannot contain more than 255 arguments"
                        );
                    }

                    arguments.push_back(parseExpression());
                } while (match({ TokenKind::COMMA }));
            }

            auto end = consume(TokenKind::RIGHT_PAREN, "expected `)` at end of argument list");
            expression = std::make_unique<FunctionCallExpression>(
                std::move(expression),
                end,
                std::move(arguments)
            );
        } else if (match({ TokenKind::DOT })) {
            auto name = consume(TokenKind::IDENTIFIER, "expected property name after `.`");
            expression = std::make_unique<GetPropertyExpression>(std::move(expression), name);
        } else if (match({ TokenKind::INCREMENT, TokenKind::DECREMENT })) {
            const auto operatorSymbol = previous();
            return std::make_unique<UpdateExpression>(
                operatorSymbol,
                std::move(expression)
            );
        } else {
            break;
        }
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parsePrimaryExpression() {
    if (match({TokenKind::TRUE})) return std::make_unique<LiteralExpression>(true);
    if (match({TokenKind::FALSE})) return std::make_unique<LiteralExpression>(false);
    if (match({TokenKind::NIL})) return std::make_unique<LiteralExpression>(NIL{});

    if (match({TokenKind::NUMBER, TokenKind::STRING, TokenKind::CHARACTER})) {
        return std::make_unique<LiteralExpression>(previous().literal);
    }

    if (match({ TokenKind::F_STRING_START })) {
        std::vector<std::unique_ptr<Expr>> elements;

        while (!check(TokenKind::F_STRING_END) && !isReachedEnd()) {
            if (match({ TokenKind::F_STRING_SLICE })) {
                elements.push_back(std::make_unique<LiteralExpression>(previous().literal));
            } else {
                consume(TokenKind::LEFT_BRACE, "expected `{` before interpolated expression");
                elements.push_back(parseExpression());
                consume(TokenKind::RIGHT_BRACE, "expected `}` at end of interpolated expression");
            }
        }

        consume(TokenKind::F_STRING_END, "unterminated string");
        return std::make_unique<InterpolatedStringLiteralExpression>(std::move(elements));
    }

    if (match({ TokenKind::LEFT_BRACKET })) {
        auto bracket = previous();

        if (match({ TokenKind::RIGHT_BRACKET })) {
            return std::make_unique<ArrayLiteralExpression>(bracket, std::vector<std::unique_ptr<Expr>>{});
        }

        std::vector<std::unique_ptr<Expr> > elements;

        if (!match({ TokenKind::RIGHT_BRACKET })) {
            do {
                elements.push_back(parseExpression());
            } while (match({TokenKind::COMMA}));
        }

        consume(TokenKind::RIGHT_BRACKET, "expected `]` in container literal expression");
        return std::make_unique<ArrayLiteralExpression>(bracket, std::move(elements));
    }

    if (match({ TokenKind::LEFT_BRACE })) {
        auto brace = previous();

        if (match({ TokenKind::RIGHT_BRACE })) {
            return std::make_unique<DictionaryLiteralExpression>(
                brace,
                std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>>{}
            );
        }

        auto firstElement = parseExpression();
        if (match({ TokenKind::COLON })) {
            std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;

            auto firstValue = parseExpression();
            pairs.emplace_back(std::move(firstElement), std::move(firstValue));

            while (match({ TokenKind::COMMA })) {
                if (check(TokenKind::RIGHT_BRACE)) break;

                auto key = parseExpression();
                consume(TokenKind::COLON, "missing `:` after dictionary key");
                auto value = parseExpression();
                pairs.emplace_back(std::move(key), std::move(value));
            }

            consume(TokenKind::RIGHT_BRACE, "expected `}` at end of container literal expression");
            return std::make_unique<DictionaryLiteralExpression>(brace, std::move(pairs));
        }

        std::unordered_set<std::unique_ptr<Expr>> elements;
        elements.insert(std::move(firstElement));

        while (match({ TokenKind::COMMA })) {
            if (check(TokenKind::RIGHT_BRACE)) break;
            elements.insert(parseExpression());
        }

        consume(TokenKind::RIGHT_BRACE, "expected `}` in container literal expression");
        return std::make_unique<SetLiteralExpression>(brace, std::move(elements));
    }

    if (match({TokenKind::IDENTIFIER})) return std::make_unique<VariableExpression>(previous());

    if (match({TokenKind::LEFT_PAREN})) {
        std::unique_ptr<Expr> expression = parseExpression();
        consume(TokenKind::RIGHT_PAREN, "expected `)` after expression");
        return std::make_unique<GroupingExpression>(std::move(expression));
    }

    diagnosticEngine.report(
        Diagnostic::DiagnosticKind::Error,
        peek().sourceLocation,
        "expected expression"
    );

    return nullptr;
}

bool Parser::match(const std::initializer_list<TokenKind> kinds) {
    if (std::ranges::any_of(kinds, [this](const auto &kind) { return check(kind); })) {
        advance();
        return true;
    }

    return false;
}

bool Parser::check(const TokenKind &kind) {
    if (isReachedEnd()) return false;
    return peek().kind == kind;
}

Token Parser::advance() {
    if (!isReachedEnd()) current++;
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

Token Parser::consume(const TokenKind &kind, const std::string &message) {
    if (check(kind)) [[likely]] return advance();

    diagnosticEngine.report(
        Diagnostic::DiagnosticKind::Error,
        peek().sourceLocation,
        message

    );

    return Token{
        .kind = TokenKind::ERROR,
        .lexeme = "error",
        .literal = NIL{},
        .sourceLocation = SourceLocation{}
    };
}

void Parser::synchronize() {
    advance();

    while (!isReachedEnd()) {
        if (previous().kind == TokenKind::SEMICOLON) return;

        switch (peek().kind) {
            case TokenKind::CLASS:
            case TokenKind::FUNCTION:
            case TokenKind::VAR:
            case TokenKind::FOR:
            case TokenKind::IF:
            case TokenKind::WHILE:
            case TokenKind::RETURN:
                return;
            default: advance();
        }
    }
}
