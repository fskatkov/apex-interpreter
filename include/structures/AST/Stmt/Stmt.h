#pragma once

#include "Common/Common.h"
#include "structures/AST/Expr/Expr.h"

struct Stmt {
    virtual ~Stmt() = default;
};

struct FunctionStatement : Stmt {
    Token name;
    std::vector<Token> arguments;
    std::unique_ptr<Stmt> statements;

    explicit FunctionStatement(Token name, std::vector<Token> arguments, std::unique_ptr<Stmt> statements)
        : name(std::move(name)), arguments(std::move(arguments)), statements(std::move(statements)) {  }
};

struct CaseStatement : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    explicit CaseStatement(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct SwitchStatement : Stmt {
    std::unique_ptr<Expr> condition;
    std::vector<std::unique_ptr<CaseStatement>> cases;
    std::unique_ptr<Stmt> defaultCase;

    explicit SwitchStatement(std::unique_ptr<Expr> condition, std::vector<std::unique_ptr<CaseStatement> > cases,
                             std::unique_ptr<Stmt> defaultCase)
        : condition(std::move(condition)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
    }
};

struct BreakStatement : Stmt {
    Token keyword;

    explicit BreakStatement(Token keyword)
        : keyword(std::move(keyword)) {  }
};

struct ContinueStatement : Stmt {
    Token keyword;

    explicit ContinueStatement(Token keyword)
        : keyword(std::move(keyword)) {  }
};

struct ForStatement : Stmt {
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;

    explicit ForStatement(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition,
                          std::unique_ptr<Expr> increment, std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)), condition(std::move(condition)), increment(std::move(increment)),
          body(std::move(body)) {
    }
};

struct WhileStatement : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    explicit WhileStatement(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct DoWhileStatement : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    explicit DoWhileStatement(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct ConditionalStatement : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenStatement;
    std::unique_ptr<Stmt> elseStatement;

    explicit ConditionalStatement(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenStatement,
                                  std::unique_ptr<Stmt> elseStatement)
        : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
          elseStatement(std::move(elseStatement)) {
    }
};

struct BlockStatement : Stmt {
    std::vector<std::unique_ptr<Stmt> > statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Stmt> > statements)
        : statements(std::move(statements)) {
    }
};

struct VariableStatement : Stmt {
    Token name;
    std::unique_ptr<Expr> initializer;
    bool isConst;

    explicit VariableStatement(Token name, std::unique_ptr<Expr> initializer, const bool isConst = false)
        : name(std::move(name)), initializer(std::move(initializer)), isConst(isConst) {
    }
};

struct PrintStatement : Stmt {
    std::unique_ptr<Expr> expression;

    explicit PrintStatement(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {
    }
};

struct ExpressionStatement : Stmt {
    std::unique_ptr<Expr> expression;

    explicit ExpressionStatement(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {
    }
};