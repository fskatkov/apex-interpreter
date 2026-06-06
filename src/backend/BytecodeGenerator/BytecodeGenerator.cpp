#include "backend/BytecodeGenerator/BytecodeGenerator.h"

BytecodeGenerator::BytecodeGenerator(DiagnosticEngine &diagnosticEngine)
    : buffer(nullptr), diagnosticEngine(diagnosticEngine), lexer(nullptr), parser(nullptr) {  }

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

void BytecodeGenerator::compileStatement(Statement *statement) {
    if (const auto *expressionStatement = dynamic_cast<ExpressionStatement *>(statement)) {
        compileExpressionStatement(expressionStatement);
    } else if (auto *variableStatement = dynamic_cast<VariableStatement *>(statement)) {
        compileVariableStatement(variableStatement);
    } else if (auto *blockStatement = dynamic_cast<BlockStatement*>(statement)) {
        compileBlockStatement(blockStatement);
    } else if (auto *conditionalStatement = dynamic_cast<ConditionalStatement *>(statement)) {
        compileConditionalStatement(conditionalStatement);
    } else if (const auto *printStatement = dynamic_cast<PrintStatement *>(statement)) {
        compilePrintStatement(printStatement);
    }
}

void BytecodeGenerator::compileExpressionStatement(const ExpressionStatement *statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
}

void BytecodeGenerator::compileVariableStatement(VariableStatement *statement) {
    if (statement->initializer) {
        compileExpression(statement->initializer.get());
    } else {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_NIL), statement->name.sourceLocation.line);
    }

    if (scopeDepth > 0) {
        locals.emplace_back(statement->name, scopeDepth);
        return;
    }

    buffer->values.emplace_back(statement->name.lexeme);
    emitByte(
        static_cast<std::uint8_t>(statement->isConst ? InstructionType::OP_DEFINE_CONST : InstructionType::OP_DEFINE_GLOBAL),
        statement->name.sourceLocation.line
    );
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), statement->name.sourceLocation.line);
}

void BytecodeGenerator::compileBlockStatement(const BlockStatement* statement) {
    beginScope();
    for (const auto& stmt : statement->statements) {
        compileStatement(stmt.get());
    }
    endScope();
}

void BytecodeGenerator::compileConditionalStatement(const ConditionalStatement* statement) {
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

void BytecodeGenerator::compilePrintStatement(const PrintStatement *statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_PRINT), 0);
}

void BytecodeGenerator::compileExpression(Expression *originalExpression) {
    if (auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression)) {
        compileVariableExpression(variableExpression);
    } else if (auto *assignmentExpression = dynamic_cast<AssignmentExpression *>(originalExpression)) {
        compileAssignmentExpression(assignmentExpression);
    } else if (auto *compoundAssignmentExpression = dynamic_cast<CompoundAssignmentExpression *>(originalExpression)) {
        compileCompoundAssignmentExpression(compoundAssignmentExpression);
    } else if (auto *updateExpression = dynamic_cast<UpdateExpression *>(originalExpression)) {
        compileUpdateExpression(updateExpression);
    } else if (const auto *groupingExpression = dynamic_cast<GroupingExpression *>(originalExpression)) {
        compileGroupingExpression(groupingExpression);
    } else if (const auto *binaryExpression = dynamic_cast<BinaryExpression *>(originalExpression)) {
        compileBinaryExpression(binaryExpression);
    } else if (const auto *unaryExpression = dynamic_cast<UnaryExpression *>(originalExpression)) {
        compileUnaryExpression(unaryExpression);
    } else if (const auto *literalExpression = dynamic_cast<LiteralExpression *>(originalExpression)) {
        compileLiteralExpression(literalExpression);
    }
}

void BytecodeGenerator::compileAssignmentExpression(AssignmentExpression *originalExpression) {
    compileExpression(originalExpression->value.get());

    if (const auto arg = resolveLocal(originalExpression->name); arg != -1) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL), originalExpression->name.sourceLocation.line);
        emitByte(static_cast<std::uint8_t>(arg), originalExpression->name.sourceLocation.line);
    } else {
        buffer->values.emplace_back(originalExpression->name.lexeme);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL), originalExpression->name.sourceLocation.line);
        emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), originalExpression->name.sourceLocation.line);
    }
}

void BytecodeGenerator::compileCompoundAssignmentExpression(CompoundAssignmentExpression* originalExpression) {
    auto *variableExpression = dynamic_cast<VariableExpression *>(originalExpression->lhs.get());
    if (!variableExpression) {
        diagnosticEngine.report(
            Diagnostic::DiagnosticKind::Error,
            originalExpression->operatorSymbol.sourceLocation,
            "invalid target for compound assignment"
        );

        return;
    }

    const auto line = originalExpression->operatorSymbol.sourceLocation.line;

    compileVariableExpression(variableExpression);
    compileExpression(originalExpression->rhs.get());

    switch (originalExpression->operatorSymbol.kind) {
        case TokenKind::PLUS_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_ADD), line);
            break;
        }
        case TokenKind::MINUS_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MINUS), line);
            break;
        }
        case TokenKind::STAR_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_STAR), line);
            break;
        }
        case TokenKind::SLASH_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SLASH), line);
            break;
        }
        case TokenKind::MODULO_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MODULO), line);
            break;
        }
        case TokenKind::BITWISE_AND_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
            break;
        }
        case TokenKind::BITWISE_OR_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
            break;
        }
        case TokenKind::BITWISE_XOR_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
            break;
        }
        case TokenKind::BITWISE_LEFT_SHIFT_EQUALS: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
            break;
        }
        case TokenKind::BITWISE_RIGHT_SHIFT_EQUALS: {
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
}

void BytecodeGenerator::compileUpdateExpression(UpdateExpression* originalExpression) {
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
    emitByte(static_cast<std::uint8_t>(isIncrement ? InstructionType::OP_ADD : InstructionType::OP_MINUS), line);

    if (const auto arg = resolveLocal(variableExpression->name); arg != -1) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL), line);
        emitByte(static_cast<std::uint8_t>(arg), line);
    } else {
        buffer->values.emplace_back(variableExpression->name.lexeme);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL), line);
        emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), line);
    }
}

void BytecodeGenerator::compileVariableExpression(VariableExpression *originalExpression) {
    if (const auto arg = resolveLocal(originalExpression->name); arg != -1) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_GET_LOCAL), originalExpression->name.sourceLocation.line);
        emitByte(static_cast<std::uint8_t>(arg), originalExpression->name.sourceLocation.line);
    } else {
        buffer->values.emplace_back(originalExpression->name.lexeme);
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL), originalExpression->name.sourceLocation.line);
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
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MINUS), line);
            break;
        }
        case TokenKind::STAR: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_STAR), line);
            break;
        }
        case TokenKind::SLASH: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_SLASH), line);
            break;
        }
        case TokenKind::MODULO: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_MODULO), line);
            break;
        }
        case TokenKind::POWER: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_POWER), line);
            break;
        }
        case TokenKind::BITWISE_AND: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
            break;
        }
        case TokenKind::BITWISE_OR: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
            break;
        }
        case TokenKind::BITWISE_XOR: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
            break;
        }
        case TokenKind::BITWISE_LEFT_SHIFT: {
            emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
            break;
        }
        case TokenKind::BITWISE_RIGHT_SHIFT: {
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
    } else if (originalExpression->unaryOperator.kind == TokenKind::BITWISE_NOT) {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT), line);
    }
}

void BytecodeGenerator::compileLiteralExpression(const LiteralExpression *originalExpression) const {
    buffer->insert(originalExpression->value, 1);
}

void BytecodeGenerator::emitByte(const std::uint8_t &byte, const std::size_t &line) const {
    buffer->update(byte, line);
}

int BytecodeGenerator::emitJump(const std::uint8_t& instruction) const {
    emitByte(instruction, 0);
    emitByte(0xff, 0);
    emitByte(0xff, 0);
    return static_cast<int>(buffer->code.size() - 2);
}

void BytecodeGenerator::patchJump(const int& offset) const {
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

int BytecodeGenerator::resolveLocal(Token& name) {
    for (int i = static_cast<int>(locals.size() - 1); i >= 0; --i) {
        if (locals[i].name.lexeme == name.lexeme) {
            return i;
        }
    }
    return -1;
}
