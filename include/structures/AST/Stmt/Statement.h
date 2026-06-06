#pragma once

#include "Common/Common.h"
#include "structures/AST/Expr/Expression.h"

struct Statement {
    virtual ~Statement() = default;
};

struct ExpressionStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit ExpressionStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {  }
};

struct PrintStatement : Statement {
    std::unique_ptr<Expression> expression;

    explicit PrintStatement(std::unique_ptr<Expression> expression)
        : expression(std::move(expression)) {  }
};

struct VariableStatement : Statement {
    Token name;
    std::unique_ptr<Expression> initializer;
    bool isConst;

    explicit VariableStatement(Token name, std::unique_ptr<Expression> initializer, const bool isConst = false)
        : name(std::move(name)), initializer(std::move(initializer)), isConst(isConst) {  }
};

struct BlockStatement : Statement {
    std::vector<std::unique_ptr<Statement> > statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement> > statements)
        : statements(std::move(statements)) {  }
};

struct ConditionalStatement : Statement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> thenStatement;
    std::unique_ptr<Statement> elseStatement;

    explicit ConditionalStatement(std::unique_ptr<Expression> condition, std::unique_ptr<Statement> thenStatement,
                                  std::unique_ptr<Statement> elseStatement)
        : condition(std::move(condition)), thenStatement(std::move(thenStatement)),
          elseStatement(std::move(elseStatement)) {
    }
};
