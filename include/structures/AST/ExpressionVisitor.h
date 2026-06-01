#pragma once

#include "Common/Common.h"

struct LiteralExpression;
struct UnaryExpression;
struct BinaryExpression;
struct GroupingExpression;

struct ExpressionVisitor {
    virtual ~ExpressionVisitor() = default;

    virtual std::any visitLiteralExpression(LiteralExpression* expression) = 0;
    virtual std::any visitUnaryExpression(UnaryExpression* expression) = 0;
    virtual std::any visitBinaryExpression(BinaryExpression* expression) = 0;
    virtual std::any visitGroupingExpression(GroupingExpression* expression) = 0;
};
