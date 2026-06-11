#pragma once

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
    std::unique_ptr<Expression> lhs;
    Token equalsToken;
    std::unique_ptr<Expression> rhs;

    explicit AssignmentExpression(std::unique_ptr<Expression> lhs, Token equalsToken, std::unique_ptr<Expression> rhs)
        : lhs(std::move(lhs)), equalsToken(std::move(equalsToken)), rhs(std::move(rhs)) {  }
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
    Value value;

    explicit LiteralExpression(Value value)
        : value(std::move(value)) {  }
};

struct InterpolatedStringLiteralExpression : Expression {
    std::vector<std::unique_ptr<Expression>> elements;

    explicit InterpolatedStringLiteralExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {  }
};

struct ArrayLiteralExpression : Expression {
    std::vector<std::unique_ptr<Expression>> elements;

    explicit ArrayLiteralExpression(std::vector<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {  }
};

struct SetLiteralExpression : Expression {
    std::unordered_set<std::unique_ptr<Expression>> elements;

    explicit SetLiteralExpression(std::unordered_set<std::unique_ptr<Expression>> elements)
        : elements(std::move(elements)) {  }
};

struct DictionaryLiteralExpression : Expression {
    std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> pairs;

    explicit DictionaryLiteralExpression(std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> pairs)
        : pairs(std::move(pairs)) {  }
};

struct IndexExpression : Expression {
    std::unique_ptr<Expression> target;
    Token bracket;
    std::unique_ptr<Expression> index;

    explicit IndexExpression(std::unique_ptr<Expression> target, Token bracket, std::unique_ptr<Expression> index)
        : target(std::move(target)), bracket(std::move(bracket)), index(std::move(index)) {  }
};