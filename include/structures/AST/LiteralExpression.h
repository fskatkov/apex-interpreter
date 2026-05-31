#pragma once

#include "Common/Common.h"
#include "Expression.h"

struct LiteralExpression : public Expression {
    std::any value;

    explicit LiteralExpression(std::any value)
        : value(std::move(value)) {  }
};
