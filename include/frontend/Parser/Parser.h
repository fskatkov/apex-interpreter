#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/AST/Expr/Expr.h"
#include "structures/AST/Stmt/Stmt.h"
#include "diagnostics/DiagnosticEngine.h"

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, DiagnosticEngine& diagnostic_engine);
    [[nodiscard]] std::vector<std::unique_ptr<Stmt>> parse();
private:
    DiagnosticEngine& diagnostic_engine;
    std::vector<Token> tokens;
    std::size_t current{0};

    [[nodiscard]] std::unique_ptr<Stmt> parse_declaration_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_function_declarationStatement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_block_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_variable_declaration_statement(bool isConst);
    [[nodiscard]] std::unique_ptr<Stmt> parse_switch_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_for_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_while_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_do_while_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_conditional_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_expression_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_break_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_continue_statement();
    [[nodiscard]] std::unique_ptr<Stmt> parse_return_statement();

    [[nodiscard]] std::unique_ptr<Expr> parse_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_assignment_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_ternary_operator_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_logical_or_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_logical_and_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_pipe_operation();
    [[nodiscard]] std::unique_ptr<Expr> parse_caret_operation();
    [[nodiscard]] std::unique_ptr<Expr> parse_ampersand_operation();
    [[nodiscard]] std::unique_ptr<Expr> parse_equality_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_comparison_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_angle_operation();
    [[nodiscard]] std::unique_ptr<Expr> parse_term_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_factor_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_exponential_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_unary_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_postfix_expression();
    [[nodiscard]] std::unique_ptr<Expr> parse_primary_expression();

    [[nodiscard]] bool match(std::initializer_list<TokenKind> kinds);
    [[nodiscard]] bool check(const TokenKind& kind);
    Token advance();
    [[nodiscard]] bool is_reached_end();
    [[nodiscard]] Token peek();
    [[nodiscard]] Token previous();
    Token consume(const TokenKind& kind, const std::string& message);
};
