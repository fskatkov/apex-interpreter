#pragma once

#include "Common/Common.h"
#include "Expression.h"

struct GroupingExpression : public Expression {
    std::unique_ptr<Expression> expression;

    explicit GroupingExpression(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {  }
};
