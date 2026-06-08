#pragma once

#include "Common/Common.h"
#include "structures/AST/Expr/Expression.h"

struct Statement {
    virtual ~Statement() = default;
};

struct CaseStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    explicit CaseStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct SwitchStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<CaseStatement>> cases;
    std::unique_ptr<Statement> defaultCase;

    explicit SwitchStatement(std::unique_ptr<Expression> condition, std::vector<std::unique_ptr<CaseStatement> > cases,
                             std::unique_ptr<Statement> defaultCase)
        : condition(std::move(condition)), cases(std::move(cases)), defaultCase(std::move(defaultCase)) {
    }
};

struct ForStatement : Statement {
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<Statement> body;

    explicit ForStatement(std::unique_ptr<Statement> initializer, std::unique_ptr<Expression> condition,
                          std::unique_ptr<Expression> increment, std::unique_ptr<Statement> body)
        : initializer(std::move(initializer)), condition(std::move(condition)), increment(std::move(increment)),
          body(std::move(body)) {
    }
};

struct WhileStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    explicit WhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct DoWhileStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    explicit DoWhileStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> body)
        : condition(std::move(condition)), body(std::move(body)) {
    }
};

struct ConditionalStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenStatement;
    std::unique_ptr<Statement> elseStatement;

    explicit ConditionalStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenStatement,
                                  std::unique_ptr<Statement> elseStatement)
        : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
          elseStatement(std::move(elseStatement)) {
    }
};

struct BlockStatement : Statement {
    std::vector<std::unique_ptr<Statement> > statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement> > statements)
        : statements(std::move(statements)) {
    }
};

struct VariableStatement : Statement {
    Token name;
    std::unique_ptr<Expression> initializer;
    bool isConst;

    explicit VariableStatement(Token name, std::unique_ptr<Expression> initializer, const bool isConst = false)
        : name(std::move(name)), initializer(std::move(initializer)), isConst(isConst) {
    }
};

struct PrintStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit PrintStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {
    }
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {
    }
};
