#pragma once

#include "Common/Common.h"
#include "structures/AST/Expr/Expression.h"

struct Statement {
    virtual ~Statement() = default;
};

struct ExpressionStatement : public Statement {
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) { }
};

struct PrintStatement : public Statement {
    std::unique_ptr<Expression> expression;

    explicit PrintStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) { }
};

struct VariableStatement : public Statement {
    Token name;
    std::unique_ptr<Expression> initializer;
    bool isConst;

    explicit VariableStatement(Token name, std::unique_ptr<Expression> initializer, const bool isConst = false)
        : name(std::move(name)), initializer(std::move(initializer)), isConst(isConst) { }
};

struct BlockStatement : public Statement {
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> statements)
        : statements(std::move(statements)) { }
};
