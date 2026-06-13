#include "backend/BytecodeGenerator/BytecodeGenerator.h"

BytecodeGenerator::BytecodeGenerator(DiagnosticEngine &diagnosticEngine)
    : buffer(nullptr), diagnosticEngine(diagnosticEngine), lexer(nullptr), parser(nullptr) {
}

void BytecodeGenerator::generate(std::string &source) {
    lexer = std::make_shared<Lexer>(source, diagnosticEngine);
    tokens = lexer->scan();
    if (lexer->encounteredErrors()) return;

    parser = std::make_shared<Parser>(tokens, diagnosticEngine);
    statements = parser->parse();
    if (diagnosticEngine.encounteredErrors()) return;

    buffer = std::make_unique<BytecodeBuffer>();
    for (const auto &statement: statements) {
        compileStatement(statement.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_RETURN), 0);
}

void BytecodeGenerator::compileStatement(Stmt *statement) {
    if (const auto *switchStatement = dynamic_cast<SwitchStatement *>(statement)) {
        compileSwitchStatement(switchStatement);
    } else if (const auto *forStatement = dynamic_cast<ForStatement *>(statement)) {
        compileForStatement(forStatement);
    } else if (const auto *breakStatement = dynamic_cast<BreakStatement *>(statement)) {
        compileBreakStatement(breakStatement);
    } else if (const auto *continueStatement = dynamic_cast<ContinueStatement *>(statement)) {
        compileContinueStatement(continueStatement);
    } else if (const auto *whileStatement = dynamic_cast<WhileStatement *>(statement)) {
        compileWhileStatement(whileStatement);
    } else if (const auto *doWhileStatement = dynamic_cast<DoWhileStatement *>(statement)) {
        compileDoWhileStatement(doWhileStatement);
    } else if (const auto *conditionalStatement = dynamic_cast<ConditionalStatement *>(statement)) {
        compileConditionalStatement(conditionalStatement);
    } else if (const auto *blockStatement = dynamic_cast<BlockStatement *>(statement)) {
        compileBlockStatement(blockStatement);
    } else if (auto *variableStatement = dynamic_cast<VariableStatement *>(statement)) {
        compileVariableStatement(variableStatement);
    } else if (auto *functionStatement = dynamic_cast<FunctionStatement *>(statement)) {
        compileFunctionStatement(functionStatement);
    } else if (const auto *expressionStatement = dynamic_cast<ExpressionStatement *>(statement)) {
        compileExpressionStatement(expressionStatement);
    } else if (const auto *printStatement = dynamic_cast<PrintStatement *>(statement)) {
        compilePrintStatement(printStatement);
    }
}

void BytecodeGenerator::compileSwitchStatement(const SwitchStatement *statement) {
    compileExpression(statement->condition.get());

    contexts.push_back(ControlFlowContext{
        false,
        -1,
        scopeDepth,
        {},
        {},
    });

    std::vector<int> endJumps;
    for (const auto &switchStatementCase: statement->cases) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_DUPLICATE), 0);
        compileExpression(switchStatementCase->condition.get());
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS), 0);

        const auto nextCaseJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

        compileStatement(switchStatementCase->body.get());
        endJumps.push_back(emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP)));

        patchJump(nextCaseJump);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    compileStatement(statement->defaultCase.get());

    for (const auto &endJump: endJumps) {
        patchJump(endJump);
    }

    for (const auto &breakJump: contexts.back().breakJumps) {
        patchJump(breakJump);
    }

    contexts.pop_back();
}

void BytecodeGenerator::compileBreakStatement(const BreakStatement *statement) {
    if (contexts.empty()) {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            statement->keyword.sourceLocation,
            "cannot use `break` in this context"
        );

        return;
    }

    auto &currentContext = contexts.back();

    for (auto i = static_cast<int>(locals.size()) - 1; i >= 0 && locals[i].depth > currentContext.loopScopeDepth; --i) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    }

    const auto breakJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));
    currentContext.breakJumps.push_back(breakJump);
}

void BytecodeGenerator::compileContinueStatement(const ContinueStatement *statement) {
    ControlFlowContext *innermostContext = nullptr;

    for (auto &context: std::views::reverse(contexts)) {
        if (context.isLoop) {
            innermostContext = &context;
            break;
        }
    }

    if (!innermostContext) {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            statement->keyword.sourceLocation,
            "cannot use `continue` in this context"
        );

        return;
    }

    for (auto i = static_cast<int>(locals.size()) - 1; i >= 0 && locals[i].depth > innermostContext->loopScopeDepth; --
         i) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    }

    if (innermostContext->continueTarget == -1) {
        innermostContext->continueJumps.push_back(emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP)));
    } else {
        emitLoop(innermostContext->continueTarget);
    }
}

void BytecodeGenerator::compileForStatement(const ForStatement *statement) {
    beginScope();
    compileStatement(statement->initializer.get());

    auto startingPoint = static_cast<int>(buffer->code.size());

    int exitJump = -1;
    if (statement->condition) {
        compileExpression(statement->condition.get());
        exitJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    }

    if (statement->increment) {
        const auto bodyJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));
        const auto increment = static_cast<int>(buffer->code.size());

        compileExpression(statement->increment.get());

        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
        emitLoop(startingPoint);
        startingPoint = increment;
        patchJump(bodyJump);
    }

    contexts.push_back(ControlFlowContext{
        true,
        startingPoint,
        scopeDepth,
        {},
        {}
    });

    compileStatement(statement->body.get());
    emitLoop(startingPoint);

    if (exitJump != -1) {
        patchJump(exitJump);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    }

    for (const auto &breakJump: contexts.back().breakJumps) {
        patchJump(breakJump);
    }

    contexts.pop_back();

    endScope();
}

void BytecodeGenerator::compileWhileStatement(const WhileStatement *statement) {
    const auto startingPoint = static_cast<int>(buffer->code.size());

    contexts.push_back(ControlFlowContext{
        true,
        startingPoint,
        scopeDepth,
        {},
        {}
    });

    compileExpression(statement->condition.get());

    const auto exitJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

    compileStatement(statement->body.get());
    emitLoop(startingPoint);

    patchJump(exitJump);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

    for (const auto &breakJump: contexts.back().breakJumps) {
        patchJump(breakJump);
    }

    contexts.pop_back();
}

void BytecodeGenerator::compileDoWhileStatement(const DoWhileStatement *statement) {
    const auto startingPoint = static_cast<int>(buffer->code.size());

    contexts.push_back(ControlFlowContext{
        true,
        -1,
        scopeDepth,
        {},
        {}
    });

    compileStatement(statement->body.get());

    for (const auto &continueJump: contexts.back().continueJumps) {
        patchJump(continueJump);
    }

    compileExpression(statement->condition.get());

    const auto exitJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    emitLoop(startingPoint);

    patchJump(exitJump);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

    for (const auto &breakJump: contexts.back().breakJumps) {
        patchJump(breakJump);
    }

    contexts.pop_back();
}

void BytecodeGenerator::compileConditionalStatement(const ConditionalStatement *statement) {
    compileExpression(statement->condition.get());

    const auto thenJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    compileStatement(statement->thenStatement.get());

    const auto elseJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));

    patchJump(thenJump);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

    if (statement->elseStatement) {
        compileStatement(statement->elseStatement.get());
    }

    patchJump(elseJump);
}

void BytecodeGenerator::compileBlockStatement(const BlockStatement *statement) {
    beginScope();
    for (const auto &stmt: statement->statements) {
        compileStatement(stmt.get());
    }
    endScope();
}

void BytecodeGenerator::compileVariableStatement(VariableStatement *statement) {
    if (statement->initializer) {
        compileExpression(statement->initializer.get());
    } else {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_NULL), statement->name.sourceLocation.line);
    }

    if (scopeDepth > 0) {
        locals.emplace_back(statement->name, scopeDepth);
        return;
    }

    buffer->values.emplace_back(statement->name.lexeme);
    emitByte(
        static_cast<std::uint8_t>(statement->isConst
                                      ? InstructionType::OP_DEFINE_CONST
                                      : InstructionType::OP_DEFINE_GLOBAL),
        statement->name.sourceLocation.line
    );
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), statement->name.sourceLocation.line);
}

void BytecodeGenerator::compileFunctionStatement(FunctionStatement* statement) {
    const auto line = statement->name.sourceLocation.line;

    const auto overBodyJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));
    const auto startingAddress = static_cast<int>(buffer->code.size());

    beginScope();

    for (const auto &arg : statement->arguments) {
        locals.emplace_back(arg, scopeDepth);
    }

    compileStatement(statement->statements.get());

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_NULL), 0);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_RETURN), 0);

    endScope();

    patchJump(overBodyJump);

    buffer->values.emplace_back(std::make_shared<Function>(
        statement->name.lexeme,
        static_cast<int>(statement->arguments.size()),
        startingAddress
    ));
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_CONSTANT), line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);

    buffer->values.emplace_back(statement->name.lexeme);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL), line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);
}

void BytecodeGenerator::compileExpressionStatement(const ExpressionStatement *statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
}

void BytecodeGenerator::compilePrintStatement(const PrintStatement *statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_PRINT), 0);
}

void BytecodeGenerator::compileExpression(Expr *originalExpression) {
    if (auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression)) {
        compileVariableExpression(variableExpression);
    } else if (const auto *assignmentExpression = dynamic_cast<AssignmentExpression *>(originalExpression)) {
        compileAssignmentExpression(assignmentExpression);
    } else if (const auto *ternaryOperatorExpression = dynamic_cast<TernaryOperatorExpression *>(originalExpression)) {
        compileTernaryOperatorExpression(ternaryOperatorExpression);
    } else if (const auto *logicalExpression = dynamic_cast<LogicalExpression *>(originalExpression)) {
        compileLogicalExpression(logicalExpression);
    } else if (const auto *compoundAssignmentExpression = dynamic_cast<CompoundAssignmentExpression *>(
        originalExpression)) {
        compileCompoundAssignmentExpression(compoundAssignmentExpression);
    } else if (const auto *updateExpression = dynamic_cast<UpdateExpression *>(originalExpression)) {
        compileUpdateExpression(updateExpression);
    } else if (const auto *groupingExpression = dynamic_cast<GroupingExpression *>(originalExpression)) {
        compileGroupingExpression(groupingExpression);
    } else if (const auto *binaryExpression = dynamic_cast<BinaryExpression *>(originalExpression)) {
        compileBinaryExpression(binaryExpression);
    } else if (const auto *unaryExpression = dynamic_cast<UnaryExpression *>(originalExpression)) {
        compileUnaryExpression(unaryExpression);
    } else if (const auto *literalExpression = dynamic_cast<LiteralExpression *>(originalExpression)) {
        compileLiteralExpression(literalExpression);
    } else if (const auto *interpolatedStringLiteralExpression = dynamic_cast<InterpolatedStringLiteralExpression *>(originalExpression)) {
        compileInterpolatedStringLiteralExpression(interpolatedStringLiteralExpression);
    } else if (const auto *arrayLiteralExpression = dynamic_cast<ArrayLiteralExpression *>(originalExpression)) {
        compileArrayLiteralExpression(arrayLiteralExpression);
    } else if (const auto *setLiteralExpression = dynamic_cast<SetLiteralExpression *>(originalExpression)) {
        compileSetLiteralExpression(setLiteralExpression);
    } else if (const auto *dictionaryLiteralExpression = dynamic_cast<DictionaryLiteralExpression *>(originalExpression)) {
        compileDictionaryLiteralExpression(dictionaryLiteralExpression);
    } else if (const auto *functionCallExpression = dynamic_cast<FunctionCallExpression *>(originalExpression)) {
        compileFunctionCallExpression(functionCallExpression);
    } else if (const auto *indexExpression = dynamic_cast<IndexExpression *>(originalExpression)) {
        compileIndexExpression(indexExpression);
    }
}

void BytecodeGenerator::compileAssignmentExpression(const AssignmentExpression *originalExpression) {
    if (auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression->lhs.get())) {
        compileExpression(originalExpression->rhs.get());

        if (const auto arg = resolveLocal(variableExpression->name); arg != -1) {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL),
                     variableExpression->name.sourceLocation.line);
            emitByte(static_cast<std::uint8_t>(arg), variableExpression->name.sourceLocation.line);
        } else {
            buffer->values.emplace_back(variableExpression->name.lexeme);
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL),
                     variableExpression->name.sourceLocation.line);
            emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1),
                     variableExpression->name.sourceLocation.line);
        }
    } else if (auto *indexExpression = dynamic_cast<IndexExpression *>(originalExpression->lhs.get())) {
        compileExpression(indexExpression->target.get());
        compileExpression(indexExpression->index.get());
        compileExpression(originalExpression->rhs.get());
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_INDEX_SET),
                 originalExpression->equalsToken.sourceLocation.line);
    }
}

void BytecodeGenerator::compileTernaryOperatorExpression(const TernaryOperatorExpression* originalExpression) {
    compileExpression(originalExpression->condition.get());

    const auto thenJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    compileExpression(originalExpression->thenBranch.get());

    const auto elseJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));

    patchJump(thenJump);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

    compileExpression(originalExpression->elseBranch.get());

    patchJump(elseJump);
}

void BytecodeGenerator::compileLogicalExpression(const LogicalExpression *originalExpression) {
    compileExpression(originalExpression->lhs.get());

    if (originalExpression->operatorSymbol.kind == TokenKind::AND) {
        const auto endJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
        compileExpression(originalExpression->rhs.get());
        patchJump(endJump);
    } else if (originalExpression->operatorSymbol.kind == TokenKind::OR) {
        const auto elseJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE));
        const auto endJump = emitJump(static_cast<std::uint8_t>(InstructionType::OP_JUMP));

        patchJump(elseJump);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);

        compileExpression(originalExpression->rhs.get());
        patchJump(endJump);
    }
}

void BytecodeGenerator::compileCompoundAssignmentExpression(const CompoundAssignmentExpression *originalExpression) {
    const auto line = originalExpression->operatorSymbol.sourceLocation.line;

    if (auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression->lhs.get())) {
        compileVariableExpression(variableExpression);
        compileExpression(originalExpression->rhs.get());

        switch (originalExpression->operatorSymbol.kind) {
            case TokenKind::PLUS_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_ADD), line);
                break;
            }
            case TokenKind::MINUS_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_SUB), line);
                break;
            }
            case TokenKind::STAR_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_MUL), line);
                break;
            }
            case TokenKind::SLASH_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_DIV), line);
                break;
            }
            case TokenKind::MODULO_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_MOD), line);
                break;
            }
            case TokenKind::AMPERSAND_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
                break;
            }
            case TokenKind::PIPE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
                break;
            }
            case TokenKind::CARET_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
                break;
            }
            case TokenKind::LEFT_ANGLE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
                break;
            }
            case TokenKind::RIGHT_ANGLE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT), line);
                break;
            }
            default:
                break;
        }

        if (const auto arg = resolveLocal(variableExpression->name); arg != -1) {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL), line);
            emitByte(static_cast<std::uint8_t>(arg), line);
        } else {
            buffer->values.emplace_back(variableExpression->name.lexeme);
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL), line);
            emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);
        }
    } else if (const auto *indexExpression = dynamic_cast<IndexExpression *>(originalExpression->lhs.get())) {
        compileExpression(indexExpression->target.get());
        compileExpression(indexExpression->index.get());

        emitByte(static_cast<std::uint8_t>(InstructionType::OP_DUPLICATE2), line);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_INDEX_GET), line);

        compileExpression(originalExpression->rhs.get());

        switch (originalExpression->operatorSymbol.kind) {
            case TokenKind::PLUS_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_ADD), line);
                break;
            }
            case TokenKind::MINUS_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_SUB), line);
                break;
            }
            case TokenKind::STAR_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_MUL), line);
                break;
            }
            case TokenKind::SLASH_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_DIV), line);
                break;
            }
            case TokenKind::MODULO_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_MOD), line);
                break;
            }
            case TokenKind::AMPERSAND_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
                break;
            }
            case TokenKind::PIPE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
                break;
            }
            case TokenKind::CARET_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
                break;
            }
            case TokenKind::LEFT_ANGLE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
                break;
            }
            case TokenKind::RIGHT_ANGLE_EQUALS: {
                emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT), line);
                break;
            }
            default:
                break;
        }

        emitByte(static_cast<std::uint8_t>(InstructionType::OP_INDEX_SET), line);
    } else {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            originalExpression->operatorSymbol.sourceLocation,
            "invalid target for compound assignment"
        );
    }
}

void BytecodeGenerator::compileUpdateExpression(const UpdateExpression *originalExpression) const {
    auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression->expression.get());
    if (!variableExpression) {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            originalExpression->operatorSymbol.sourceLocation,
            "target of increment/decrement must be a variable"
        );

        return;
    }

    const auto line = originalExpression->operatorSymbol.sourceLocation.line;
    const auto isIncrement = originalExpression->operatorSymbol.kind == TokenKind::INCREMENT;

    compileVariableExpression(variableExpression);
    buffer->values.emplace_back(1.0);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_CONSTANT), line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);
    emitByte(static_cast<std::uint8_t>(isIncrement ? InstructionType::OP_ADD : InstructionType::OP_SUB), line);

    if (const auto arg = resolveLocal(variableExpression->name); arg != -1) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL), line);
        emitByte(static_cast<std::uint8_t>(arg), line);
    } else {
        buffer->values.emplace_back(variableExpression->name.lexeme);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL), line);
        emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);
    }
}

void BytecodeGenerator::compileVariableExpression(VariableExpression *originalExpression) const {
    if (const auto arg = resolveLocal(originalExpression->name); arg != -1) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_GET_LOCAL),
                 originalExpression->name.sourceLocation.line);
        emitByte(static_cast<std::uint8_t>(arg), originalExpression->name.sourceLocation.line);
    } else {
        buffer->values.emplace_back(originalExpression->name.lexeme);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL),
                 originalExpression->name.sourceLocation.line);
        emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), originalExpression->name.sourceLocation.line);
    }
}

void BytecodeGenerator::compileGroupingExpression(const GroupingExpression *originalExpression) {
    compileExpression(originalExpression->expression.get());
}

void BytecodeGenerator::compileBinaryExpression(const BinaryExpression *originalExpression) {
    compileExpression(originalExpression->lhs.get());
    compileExpression(originalExpression->rhs.get());

    const auto line = originalExpression->binaryOperator.sourceLocation.line;
    switch (originalExpression->binaryOperator.kind) {
        case TokenKind::PLUS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_ADD), line);
            break;
        }
        case TokenKind::MINUS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SUB), line);
            break;
        }
        case TokenKind::STAR: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MUL), line);
            break;
        }
        case TokenKind::SLASH: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_DIV), line);
            break;
        }
        case TokenKind::MODULO: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MOD), line);
            break;
        }
        case TokenKind::POWER: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_POW), line);
            break;
        }
        case TokenKind::AMPERSAND: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
            break;
        }
        case TokenKind::PIPE: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
            break;
        }
        case TokenKind::CARET: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
            break;
        }
        case TokenKind::LEFT_ANGLE: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
            break;
        }
        case TokenKind::RIGHT_ANGLE: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT), line);
            break;
        }
        case TokenKind::EQUALS_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS), line);
            break;
        }
        case TokenKind::GREATER: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_GREATER), line);
            break;
        }
        case TokenKind::GREATER_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_GREATER_EQUALS), line);
            break;
        }
        case TokenKind::LESS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_LESS), line);
            break;
        }
        case TokenKind::LESS_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_LESS_EQUALS), line);
            break;
        }
        case TokenKind::IN: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_IN), line);
            break;
        }
        default:
            break;
    }
}

void BytecodeGenerator::compileUnaryExpression(const UnaryExpression *originalExpression) {
    compileExpression(originalExpression->expression.get());

    const auto line = originalExpression->unaryOperator.sourceLocation.line;
    if (originalExpression->unaryOperator.kind == TokenKind::BANG) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_NOT), line);
    } else if (originalExpression->unaryOperator.kind == TokenKind::MINUS) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_NEGATE), line);
    } else if (originalExpression->unaryOperator.kind == TokenKind::TILDE) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT), line);
    } else if (originalExpression->unaryOperator.kind == TokenKind::TYPEOF) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_TYPEOF), line);
    }
}

void BytecodeGenerator::compileLiteralExpression(const LiteralExpression *originalExpression) const {
    buffer->insert(originalExpression->value, 1);
}

void BytecodeGenerator::compileInterpolatedStringLiteralExpression(const InterpolatedStringLiteralExpression* originalExpression) {
    for (const auto& element : originalExpression->elements) {
        compileExpression(element.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_BUILD_STRING), 0);

    const auto count = originalExpression->elements.size();
    emitByte(static_cast<std::uint8_t>((count >> 8) & 0xff), 0);
    emitByte(static_cast<std::uint8_t>(count & 0xff), 0);
}

void BytecodeGenerator::compileArrayLiteralExpression(const ArrayLiteralExpression *originalExpression) {
    for (const auto &element: originalExpression->elements) {
        compileExpression(element.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_BUILD_ARRAY), 0);

    const auto count = originalExpression->elements.size();
    emitByte(static_cast<std::uint8_t>((count >> 8) & 0xff), 0);
    emitByte(static_cast<std::uint8_t>(count & 0xff), 0);
}

void BytecodeGenerator::compileSetLiteralExpression(const SetLiteralExpression* originalExpression) {
    for (const auto &element : originalExpression->elements) {
        compileExpression(element.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_BUILD_SET), 0);

    const auto count = originalExpression->elements.size();
    emitByte(static_cast<std::uint8_t>((count >> 8) & 0xff), 0);
    emitByte(static_cast<std::uint8_t>(count & 0xff), 0);
}

void BytecodeGenerator::compileDictionaryLiteralExpression(const DictionaryLiteralExpression* statement) {
    for (const auto&[first, second] : statement->pairs) {
        compileExpression(first.get());
        compileExpression(second.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_BUILD_DICTIONARY), 0);

    const auto count = statement->pairs.size();
    emitByte(static_cast<std::uint8_t>((count >> 8) & 0xff), 0);
    emitByte(static_cast<std::uint8_t>(count & 0xff), 0);
}

void BytecodeGenerator::compileFunctionCallExpression(const FunctionCallExpression* originalExpression) {
    compileExpression(originalExpression->callee.get());
    for (const auto &arg : originalExpression->arguments) {
        compileExpression(arg.get());
    }
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_CALL), originalExpression->end.sourceLocation.line);
    emitByte(static_cast<std::uint8_t>(originalExpression->arguments.size()),
             originalExpression->end.sourceLocation.line);
}

void BytecodeGenerator::compileIndexExpression(const IndexExpression *originalExpression) {
    compileExpression(originalExpression->target.get());
    compileExpression(originalExpression->index.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_INDEX_GET), originalExpression->bracket.sourceLocation.line);
}

void BytecodeGenerator::emitByte(const std::uint8_t &byte, const std::size_t &line) const {
    buffer->update(byte, line);
}

int BytecodeGenerator::emitJump(const std::uint8_t &instruction) const {
    emitByte(instruction, 0);
    emitByte(0xff, 0);
    emitByte(0xff, 0);
    return static_cast<int>(buffer->code.size() - 2);
}

void BytecodeGenerator::emitLoop(const int &startingPoint) const {
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_LOOP), 0);
    const auto offset = static_cast<int>(buffer->code.size()) - startingPoint + 2;
    emitByte(offset >> 8 & 0xff, 0);
    emitByte(offset & 0xff, 0);
}

void BytecodeGenerator::patchJump(const int &offset) const {
    const auto jump = static_cast<int>(buffer->code.size()) - offset - 2;
    buffer->code[offset] = jump >> 8 & 0xff;
    buffer->code[offset + 1] = jump & 0xff;
}

void BytecodeGenerator::beginScope() {
    scopeDepth++;
}

void BytecodeGenerator::endScope() {
    scopeDepth--;

    while (!locals.empty() && locals.back().depth > scopeDepth) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
        locals.pop_back();
    }
}

int BytecodeGenerator::resolveLocal(const Token &name) const {
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; --i) {
        if (locals[i].name.lexeme == name.lexeme) {
            return i;
        }
    }
    return -1;
}
