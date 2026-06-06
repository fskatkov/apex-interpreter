#pragma once

#include <utility>

#include "Common/Common.h"
#include "structures/Token/Token.h"

struct Expression {
    virtual ~Expression() = default;
};

struct VariableExpression : Expression {
    Token name;

    explicit VariableExpression(Token name)
        : name(std::move(name)) {  }
};

struct AssignmentExpression : Expression {
    Token name;
    std::unique_ptr<Expression> value;

    explicit AssignmentExpression(Token  name, std::unique_ptr<Expression> value)
        : name(std::move(name)), value(std::move(value)) {  }
};

struct CompoundAssignmentExpression : Expression {
    std::unique_ptr<Expression> lhs;
    Token operatorSymbol;
    std::unique_ptr<Expression> rhs;

    explicit CompoundAssignmentExpression(std::unique_ptr<Expression> lhs, Token operatorSymbol, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), operatorSymbol(std::move(operatorSymbol)), rhs(std::move(rhs)) {  }
};

struct LogicalExpression : Expression {
    std::unique_ptr<Expression> lhs;
    Token operatorSymbol;
    std::unique_ptr<Expression> rhs;

    explicit LogicalExpression(std::unique_ptr<Expression> lhs, Token operatorSymbol, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), operatorSymbol(std::move(operatorSymbol)), rhs(std::move(rhs)) {  }
};

struct GroupingExpression : Expression {
    std::unique_ptr<Expression> expression;

    explicit GroupingExpression(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {  }
};

struct BinaryExpression : Expression {
    std::unique_ptr<Expression> lhs;
    Token binaryOperator;
    std::unique_ptr<Expression> rhs;

    explicit BinaryExpression(std::unique_ptr<Expression> lhs, Token  binaryOperator, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), binaryOperator(std::move(binaryOperator)), rhs(std::move(rhs)) {  }
};

struct UnaryExpression : Expression {
    Token unaryOperator;
    std::unique_ptr<Expression> expression;

    explicit UnaryExpression(Token  unaryOperator, std::unique_ptr<Expression> expression)
        : unaryOperator(std::move(unaryOperator)), expression(std::move(expression)) {  }
};

struct UpdateExpression : Expression {
    Token operatorSymbol;
    std::unique_ptr<Expression> expression;

    explicit UpdateExpression(Token operatorSymbol, std::unique_ptr<Expression> expression)
        : operatorSymbol(std::move(operatorSymbol)), expression(std::move(expression)) {  }
};

struct LiteralExpression : Expression {
    std::any value;

    explicit LiteralExpression(std::any value)
        : value(std::move(value)) {  }
};
