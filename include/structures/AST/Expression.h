#pragma once

#include "ExprVisitor.h"

struct Expression {
    virtual ~Expression() = default;
    virtual std::any accept(ExpressionVisitor& visitor) = 0;
};
