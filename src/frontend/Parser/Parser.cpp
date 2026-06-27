#include "frontend/Parser/Parser.h"

Parser::Parser(std::vector<Token> tokens, DiagnosticEngine &diagnostic_engine)
    : diagnostic_engine(diagnostic_engine), tokens(std::move(tokens)) {
}

std::vector<std::unique_ptr<Stmt> > Parser::parse() {
    std::vector<std::unique_ptr<Stmt> > statements;
    while (!is_reached_end()) {
        statements.push_back(parse_declaration_statement());
    }
    return statements;
}

std::unique_ptr<Stmt> Parser::parse_declaration_statement() {
    if (match({ TokenKind::FUNCTION })) return parse_function_declarationStatement();
    if (match({TokenKind::VAR})) return parse_variable_declaration_statement(false);
    if (match({TokenKind::CONST})) return parse_variable_declaration_statement(true);
    return parse_statement();
}

std::unique_ptr<Stmt> Parser::parse_statement() {
    if (match({TokenKind::SWITCH})) return parse_switch_statement();
    if (match({TokenKind::BREAK})) return parse_break_statement();
    if (match({TokenKind::CONTINUE})) return parse_continue_statement();
    if (match({TokenKind::FOR})) return parse_for_statement();
    if (match({TokenKind::WHILE})) return parse_while_statement();
    if (match({TokenKind::DO})) return parse_do_while_statement();
    if (match({TokenKind::IF})) return parse_conditional_statement();
    if (match({TokenKind::LEFT_BRACE})) return parse_block_statement();
    if (match({TokenKind::RETURN})) return parse_return_statement();
    return parse_expression_statement();
}

std::unique_ptr<Stmt> Parser::parse_function_declarationStatement() {
    auto name = consume(TokenKind::IDENTIFIER, "expected function name");
    consume(TokenKind::LEFT_PAREN, "expected `(` before the argument list");

    std::vector<Token> arguments;
    if (!check({ TokenKind::RIGHT_PAREN })) {
        do {
            if (arguments.size() >= 255) [[unlikely]] {
                diagnostic_engine.report(
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

    auto body = parse_block_statement();
    return std::make_unique<FunctionStatement>(
        name,
        arguments,
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::parse_block_statement() {
    std::vector<std::unique_ptr<Stmt> > statements;
    while (!check(TokenKind::RIGHT_BRACE) && !is_reached_end()) {
        statements.push_back(parse_declaration_statement());
    }

    consume(TokenKind::RIGHT_BRACE, "expected `}` at end of block statement");
    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parse_variable_declaration_statement(bool isConst) {
    const auto name = consume(TokenKind::IDENTIFIER, "expected variable name");

    std::unique_ptr<Expr> initializer = nullptr;
    if (match({TokenKind::EQUALS})) {
        initializer = parse_expression();
    } else if (isConst) [[unlikely]] {
        diagnostic_engine.report(
            Diagnostic::DiagnosticKind::Error,
            name.sourceLocation,
            "const variables must be initialized"
        );
        return nullptr;
    }

    consume(TokenKind::SEMICOLON, "expected `;` at end of variable declaration");
    return std::make_unique<VariableStatement>(name, std::move(initializer), isConst);
}

std::unique_ptr<Stmt> Parser::parse_switch_statement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parse_expression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");
    consume(TokenKind::LEFT_BRACE, "expected `{` before `switch` body");

    std::vector<std::unique_ptr<CaseStatement> > cases;
    while (match({TokenKind::CASE})) {
        auto caseExpression = parse_expression();
        consume(TokenKind::COLON, "expected `:` at end of `case` condition");
        auto caseBody = parse_statement();

        cases.push_back(std::make_unique<CaseStatement>(
            std::move(caseExpression),
            std::move(caseBody)
        ));
    }

    consume(TokenKind::DEFAULT, "expected `default` case in `switch` body");
    consume(TokenKind::COLON, "expected `:` at end of `default` clause");

    auto defaultCase = parse_statement();
    consume(TokenKind::RIGHT_BRACE, "expected `}` at end of `switch` body");

    return std::make_unique<SwitchStatement>(
        std::move(condition),
        std::move(cases),
        std::move(defaultCase)
    );
}

std::unique_ptr<Stmt> Parser::parse_for_statement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");

    auto initializer = match({ TokenKind::VAR })
        ? parse_variable_declaration_statement(false)
        : parse_expression_statement();

    auto condition = check({TokenKind::SEMICOLON}) ? nullptr : parse_expression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `for` condition");

    auto increment = check({TokenKind::RIGHT_PAREN}) ? nullptr : parse_expression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto body = parse_statement();

    return std::make_unique<ForStatement>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body)
    );
}

std::unique_ptr<Stmt> Parser::parse_while_statement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parse_expression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto body = parse_statement();
    return std::make_unique<WhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_do_while_statement() {
    auto body = parse_statement();

    consume(TokenKind::WHILE, "expected `while` statement after `do` body");
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parse_expression();
    consume(TokenKind::RIGHT_PAREN, "expected `(` in expression list");
    consume(TokenKind::SEMICOLON, "expected `;` at end of `do-while`");

    return std::make_unique<DoWhileStatement>(std::move(condition), std::move(body));
}

std::unique_ptr<Stmt> Parser::parse_conditional_statement() {
    consume(TokenKind::LEFT_PAREN, "expected `(` in expression list");
    auto condition = parse_expression();
    consume(TokenKind::RIGHT_PAREN, "expected `)` in expression list");

    auto then_branch = parse_statement();
    auto else_branch = !match({ TokenKind::ELSE }) ? nullptr : parse_statement();

    return std::make_unique<ConditionalStatement>(
        std::move(condition),
        std::move(then_branch),
        std::move(else_branch)
    );
}

std::unique_ptr<Stmt> Parser::parse_expression_statement() {
    auto expression = parse_expression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of expression");
    return std::make_unique<ExpressionStatement>(std::move(expression));
}

std::unique_ptr<Stmt> Parser::parse_break_statement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `break` statement");
    return std::make_unique<BreakStatement>(keyword);
}

std::unique_ptr<Stmt> Parser::parse_continue_statement() {
    auto keyword = previous();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `continue` statement");
    return std::make_unique<ContinueStatement>(keyword);
}

std::unique_ptr<Stmt> Parser::parse_return_statement() {
    auto keyword = previous();
    auto value = check({ TokenKind::SEMICOLON }) ? nullptr : parse_expression();
    consume(TokenKind::SEMICOLON, "expected `;` at end of `return` statement");
    return std::make_unique<ReturnStatement>(std::move(keyword), std::move(value));
}

std::unique_ptr<Expr> Parser::parse_expression() {
    return parse_assignment_expression();
}

std::unique_ptr<Expr> Parser::parse_assignment_expression() {
    const int detected_multi_assignment = current;
    bool is_multi_assignment = false;

    if (detected_multi_assignment + 1 < tokens.size() && tokens[detected_multi_assignment].kind == TokenKind::IDENTIFIER
        && tokens[detected_multi_assignment + 1].kind == TokenKind::COMMA) {

        int temporary_pointer = detected_multi_assignment;
        int number_of_operands = 0;

        while (temporary_pointer < tokens.size() && tokens[temporary_pointer].kind == TokenKind::IDENTIFIER) {
            number_of_operands++;

            if (temporary_pointer + 1 < tokens.size() && tokens[temporary_pointer + 1].kind == TokenKind::COMMA) {
                temporary_pointer += 2;
            } else if (temporary_pointer + 1 < tokens.size() && tokens[temporary_pointer + 1].kind == TokenKind::EQUALS) {
                if (number_of_operands > 1) {
                    is_multi_assignment = true;
                }

                break;
            } else {
                break;
            }
        }
    }

    if (is_multi_assignment) {
        std::vector<std::string> target_variables;

        do {
            target_variables.push_back(consume(TokenKind::IDENTIFIER, "").lexeme);
        } while (match({ TokenKind::COMMA }));

        const auto equals_token = consume(TokenKind::EQUALS, "expected `=` after variables");

        std::vector<std::unique_ptr<Expr>> expressions;
        do {
            expressions.push_back(parse_ternary_operator_expression());
        } while (match({ TokenKind::COMMA }));

        if (target_variables.size() != expressions.size()) [[unlikely]] {
            diagnostic_engine.report(
                Diagnostic::DiagnosticKind::Error,
                equals_token.sourceLocation,
                "mismatch number of operands"
            );
        }

        return std::make_unique<MultiAssignmentExpression>(
            std::move(target_variables),
            std::move(expressions)
        );
    }

    auto expression = parse_ternary_operator_expression();

    if (match({ TokenKind::EQUALS })) {
        const auto operator_symbol = previous();
        auto rhs = parse_ternary_operator_expression();

        if (dynamic_cast<VariableExpression *>(expression.get()) ||
            dynamic_cast<IndexExpression *>(expression.get())) [[likely]] {

            return std::make_unique<AssignmentExpression>(
                std::move(expression),
                operator_symbol,
                std::move(rhs)
            );
        }

        diagnostic_engine.report(
            Diagnostic::DiagnosticKind::Error,
            operator_symbol.sourceLocation,
            "invalid assignment target"
        );
    }

    if (match({
        TokenKind::PLUS_EQUALS, TokenKind::MINUS_EQUALS, TokenKind::STAR_EQUALS, TokenKind::SLASH_EQUALS,
        TokenKind::MODULO_EQUALS, TokenKind::AMPERSAND_EQUALS, TokenKind::PIPE_EQUALS,
        TokenKind::CARET_EQUALS, TokenKind::LEFT_ANGLE_EQUALS, TokenKind::RIGHT_ANGLE_EQUALS
    })) {
        const auto operator_symbol = previous();
        auto rhs = parse_ternary_operator_expression();

        if (dynamic_cast<VariableExpression *>(expression.get()) ||
            dynamic_cast<IndexExpression *>(expression.get())) [[likely]] {

            return std::make_unique<CompoundAssignmentExpression>(
                std::move(expression),
                operator_symbol,
                std::move(rhs)
            );
        }

        diagnostic_engine.report(
            Diagnostic::DiagnosticKind::Error,
            operator_symbol.sourceLocation,
            "invalid assignment target"
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_ternary_operator_expression() {
    auto expression = parse_logical_or_expression();

    if (match({ TokenKind::QUESTION_MARK })) {
        auto then_branch = parse_ternary_operator_expression();
        consume(TokenKind::COLON, "expected `:` after `? ...` in ternary expression");
        auto else_branch = parse_ternary_operator_expression();
        expression = std::make_unique<TernaryOperatorExpression>(
            std::move(expression),
            std::move(then_branch),
            std::move(else_branch)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_logical_or_expression() {
    auto expression = parse_logical_and_expression();

    while (match({ TokenKind::OR })) {
        const auto operator_symbol = previous();
        auto rhs = parse_logical_and_expression();
        expression = std::make_unique<LogicalExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_logical_and_expression() {
    auto expression = parse_pipe_operation();

    while (match({ TokenKind::AND })) {
        const auto operator_symbol = previous();
        auto rhs = parse_pipe_operation();
        expression = std::make_unique<LogicalExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_pipe_operation() {
    auto expression = parse_caret_operation();

    while (match({ TokenKind::PIPE })) {
        const auto operator_symbol = previous();
        auto rhs = parse_caret_operation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_caret_operation() {
    auto expression = parse_ampersand_operation();

    while (match({ TokenKind::CARET })) {
        const auto operator_symbol = previous();
        auto rhs = parse_ampersand_operation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_ampersand_operation() {
    auto expression = parse_equality_expression();

    while (match({ TokenKind::AMPERSAND })) {
        const auto operator_symbol = previous();
        auto rhs = parse_equality_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_equality_expression() {
    auto expression = parse_comparison_expression();

    while (match({ TokenKind::BANG_EQUALS, TokenKind::EQUALS_EQUALS })) {
        const auto operator_symbol = previous();
        auto rhs = parse_comparison_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_comparison_expression() {
    auto expression = parse_angle_operation();

    while (match({
        TokenKind::GREATER, TokenKind::GREATER_EQUALS, TokenKind::LESS, TokenKind::LESS_EQUALS, TokenKind::IN
    })) {
        const auto operator_symbol = previous();
        auto rhs = parse_angle_operation();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_angle_operation() {
    auto expression = parse_term_expression();

    while (match({ TokenKind::LEFT_ANGLE, TokenKind::RIGHT_ANGLE })) {
        const auto operator_symbol = previous();
        auto rhs = parse_term_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_term_expression() {
    auto expression = parse_factor_expression();

    while (match({ TokenKind::PLUS, TokenKind::MINUS })) {
        const auto operator_symbol = previous();
        auto rhs = parse_factor_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_factor_expression() {
    auto expression = parse_exponential_expression();

    while (match({ TokenKind::MODULO, TokenKind::SLASH, TokenKind::STAR, TokenKind::POWER })) {
        const auto operator_symbol = previous();
        auto rhs = parse_exponential_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_exponential_expression() {
    auto expression = parse_unary_expression();

    while (match({ TokenKind::POWER })) {
        const auto operator_symbol = previous();
        auto rhs = parse_unary_expression();
        expression = std::make_unique<BinaryExpression>(
            std::move(expression),
            operator_symbol,
            std::move(rhs)
        );
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_unary_expression() {
    while (match({ TokenKind::BANG, TokenKind::MINUS, TokenKind::TILDE, TokenKind::TYPEOF })) {
        const auto operator_symbol = previous();
        auto rhs = parse_postfix_expression();
        return std::make_unique<UnaryExpression>(
            operator_symbol,
            std::move(rhs)
        );
    }

    return parse_postfix_expression();
}

std::unique_ptr<Expr> Parser::parse_postfix_expression() {
    auto expression = parse_primary_expression();

    while (true) {
        if (match({ TokenKind::LEFT_BRACKET })) {
            const auto bracket = previous();
            auto index = parse_expression();
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
                        diagnostic_engine.report(
                            Diagnostic::DiagnosticKind::Error,
                            peek().sourceLocation,
                            "function call cannot contain more than 255 arguments"
                        );
                    }

                    arguments.push_back(parse_expression());
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
            const auto operator_symbol = previous();
            return std::make_unique<UpdateExpression>(
                operator_symbol,
                std::move(expression)
            );
        } else {
            break;
        }
    }

    return expression;
}

std::unique_ptr<Expr> Parser::parse_primary_expression() {
    if (match({TokenKind::TRUE})) return std::make_unique<LiteralExpression>(true);
    if (match({TokenKind::FALSE})) return std::make_unique<LiteralExpression>(false);
    if (match({TokenKind::NIL})) return std::make_unique<LiteralExpression>(NIL{});

    if (match({TokenKind::NUMBER, TokenKind::STRING, TokenKind::CHARACTER})) {
        return std::make_unique<LiteralExpression>(previous().literal);
    }

    if (match({ TokenKind::F_STRING_START })) {
        std::vector<std::unique_ptr<Expr>> elements;

        while (!check(TokenKind::F_STRING_END) && !is_reached_end()) {
            if (match({ TokenKind::F_STRING_SLICE })) {
                elements.push_back(std::make_unique<LiteralExpression>(previous().literal));
            } else {
                consume(TokenKind::LEFT_BRACE, "expected `{` before interpolated expression");
                elements.push_back(parse_expression());
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
                elements.push_back(parse_expression());
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

        auto first_element = parse_expression();
        if (match({ TokenKind::COLON })) {
            std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;

            auto firstValue = parse_expression();
            pairs.emplace_back(std::move(first_element), std::move(firstValue));

            while (match({ TokenKind::COMMA })) {
                if (check(TokenKind::RIGHT_BRACE)) break;

                auto key = parse_expression();
                consume(TokenKind::COLON, "missing `:` after dictionary key");
                auto value = parse_expression();
                pairs.emplace_back(std::move(key), std::move(value));
            }

            consume(TokenKind::RIGHT_BRACE, "expected `}` at end of container literal expression");
            return std::make_unique<DictionaryLiteralExpression>(brace, std::move(pairs));
        }

        std::unordered_set<std::unique_ptr<Expr>> elements;
        elements.insert(std::move(first_element));

        while (match({ TokenKind::COMMA })) {
            if (check(TokenKind::RIGHT_BRACE)) break;
            elements.insert(parse_expression());
        }

        consume(TokenKind::RIGHT_BRACE, "expected `}` in container literal expression");
        return std::make_unique<SetLiteralExpression>(brace, std::move(elements));
    }

    if (match({TokenKind::IDENTIFIER})) return std::make_unique<VariableExpression>(previous());

    if (match({TokenKind::LEFT_PAREN})) {
        std::unique_ptr<Expr> expression = parse_expression();
        consume(TokenKind::RIGHT_PAREN, "expected `)` after expression");
        return std::make_unique<GroupingExpression>(std::move(expression));
    }

    diagnostic_engine.report(
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
    if (is_reached_end()) return false;
    return peek().kind == kind;
}

Token Parser::advance() {
    if (!is_reached_end()) current++;
    return previous();
}

bool Parser::is_reached_end() {
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

    diagnostic_engine.report(
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