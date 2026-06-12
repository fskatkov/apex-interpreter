#pragma once

#include <utility>

#include "Common/Common.h"
#include "structures/Token/Token.h"

struct Expr {
    virtual ~Expr() = default;
};

struct VariableExpression : Expr {
    Token name;

    explicit VariableExpression(Token name)
        : name(std::move(name)) {  }
};

struct AssignmentExpression : Expr {
    std::unique_ptr<Expr> lhs;
    Token equalsToken;
    std::unique_ptr<Expr> rhs;

    explicit AssignmentExpression(std::unique_ptr<Expr> lhs, Token equalsToken, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), equalsToken(std::move(equalsToken)), rhs(std::move(rhs)) {  }
};

struct CompoundAssignmentExpression : Expr {
    std::unique_ptr<Expr> lhs;
    Token operatorSymbol;
    std::unique_ptr<Expr> rhs;

    explicit CompoundAssignmentExpression(std::unique_ptr<Expr> lhs, Token operatorSymbol, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), operatorSymbol(std::move(operatorSymbol)), rhs(std::move(rhs)) {  }
};

struct LogicalExpression : Expr {
    std::unique_ptr<Expr> lhs;
    Token operatorSymbol;
    std::unique_ptr<Expr> rhs;

    explicit LogicalExpression(std::unique_ptr<Expr> lhs, Token operatorSymbol, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), operatorSymbol(std::move(operatorSymbol)), rhs(std::move(rhs)) {  }
};

struct GroupingExpression : Expr {
    std::unique_ptr<Expr> expression;

    explicit GroupingExpression(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {  }
};

struct BinaryExpression : Expr {
    std::unique_ptr<Expr> lhs;
    Token binaryOperator;
    std::unique_ptr<Expr> rhs;

    explicit BinaryExpression(std::unique_ptr<Expr> lhs, Token  binaryOperator, std::unique_ptr<Expr> rhs)
        : lhs(std::move(lhs)), binaryOperator(std::move(binaryOperator)), rhs(std::move(rhs)) {  }
};

struct UnaryExpression : Expr {
    Token unaryOperator;
    std::unique_ptr<Expr> expression;

    explicit UnaryExpression(Token  unaryOperator, std::unique_ptr<Expr> expression)
        : unaryOperator(std::move(unaryOperator)), expression(std::move(expression)) {  }
};

struct UpdateExpression : Expr {
    Token operatorSymbol;
    std::unique_ptr<Expr> expression;

    explicit UpdateExpression(Token operatorSymbol, std::unique_ptr<Expr> expression)
        : operatorSymbol(std::move(operatorSymbol)), expression(std::move(expression)) {  }
};

struct TernaryOperatorExpression : Expr {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenBranch;
    std::unique_ptr<Expr> elseBranch;

    explicit TernaryOperatorExpression(std::unique_ptr<Expr> condition,
                                       std::unique_ptr<Expr> thenBranch,
                                       std::unique_ptr<Expr> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
          elseBranch(std::move(elseBranch)) {
    }
};

struct LiteralExpression : Expr {
    Value value;

    explicit LiteralExpression(Value value)
        : value(std::move(value)) {  }
};

struct InterpolatedStringLiteralExpression : Expr {
    std::vector<std::unique_ptr<Expr>> elements;

    explicit InterpolatedStringLiteralExpression(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {  }
};

struct ArrayLiteralExpression : Expr {
    std::vector<std::unique_ptr<Expr>> elements;

    explicit ArrayLiteralExpression(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {  }
};

struct SetLiteralExpression : Expr {
    std::unordered_set<std::unique_ptr<Expr>> elements;

    explicit SetLiteralExpression(std::unordered_set<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {  }
};

struct DictionaryLiteralExpression : Expr {
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs;

    explicit DictionaryLiteralExpression(std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> pairs)
        : pairs(std::move(pairs)) {  }
};

struct FunctionCallExpression : Expr {
    std::unique_ptr<Expr> callee;
    Token end;
    std::vector<std::unique_ptr<Expr>> arguments;

    explicit FunctionCallExpression(std::unique_ptr<Expr> callee, Token end, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), end(std::move(end)), arguments(std::move(arguments)) {  }
};

struct IndexExpression : Expr {
    std::unique_ptr<Expr> target;
    Token bracket;
    std::unique_ptr<Expr> index;

    explicit IndexExpression(std::unique_ptr<Expr> target, Token bracket, std::unique_ptr<Expr> index)
        : target(std::move(target)), bracket(std::move(bracket)), index(std::move(index)) {  }
};