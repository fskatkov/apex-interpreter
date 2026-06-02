#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"

struct Expression {
    virtual ~Expression() = default;
};

struct GroupingExpression : public Expression {
    std::unique_ptr<Expression> expression;

    explicit GroupingExpression(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {  }
};

struct BinaryExpression : public Expression {
    std::unique_ptr<Expression> lhs;
    Token binaryOperator;
    std::unique_ptr<Expression> rhs;

    explicit BinaryExpression(std::unique_ptr<Expression> lhs, const Token& binaryOperator, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), binaryOperator(binaryOperator), rhs(std::move(rhs)) {  }
};

struct UnaryExpression : public Expression {
    Token unaryOperator;
    std::unique_ptr<Expression> expression;

    explicit UnaryExpression(const Token& unaryOperator, std::unique_ptr<Expression> expression)
        : unaryOperator(unaryOperator), expression(std::move(expression)) {  }
};

struct LiteralExpression : public Expression {
    std::any value;

    explicit LiteralExpression(std::any value)
        : value(std::move(value)) {  }
};
