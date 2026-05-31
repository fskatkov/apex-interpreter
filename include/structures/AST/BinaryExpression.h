#pragma once

#include "Common/Common.h"
#include "Expression.h"
#include "structures/Token/Token.h"

struct BinaryExpression : public Expression {
    std::unique_ptr<Expression> lhs;
    Token binaryOperator;
    std::unique_ptr<Expression> rhs;

    explicit BinaryExpression(std::unique_ptr<Expression> lhs, const Token& binaryOperator, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), binaryOperator(binaryOperator), rhs(std::move(rhs)) {  }
};
