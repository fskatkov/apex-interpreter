#pragma once

#include "Common/Common.h"
#include "Expression.h"
#include "structures/Token/Token.h"

struct UnaryExpression : public Expression {
    Token unaryOperator;
    std::unique_ptr<Expression> expression;

    explicit UnaryExpression(const Token& unaryOperator, std::unique_ptr<Expression> expression)
        : unaryOperator(unaryOperator), expression(std::move(expression)) {  }

    std::any accept(ExpressionVisitor& visitor) override {
        return visitor.visitUnaryExpression(this);
    }
};
