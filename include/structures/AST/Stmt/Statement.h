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
