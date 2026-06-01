#pragma once

#include "ExpressionVisitor.h"

struct Expression {
    virtual ~Expression() = default;
    virtual std::any accept(ExpressionVisitor& visitor) = 0;
};
