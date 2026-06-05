#include "backend/BytecodeGenerator/BytecodeGenerator.h"

BytecodeGenerator::BytecodeGenerator(DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), lexer(nullptr), parser(nullptr) {  }

std::unique_ptr<BytecodeBuffer> BytecodeGenerator::generate(std::string& source) {
    lexer = std::make_shared<Lexer>(source, diagnosticEngine);
    tokens = lexer->scan();
    if (lexer->encounteredErrors()) {
        return nullptr;
    }

    parser = std::make_shared<Parser>(tokens, diagnosticEngine);
    statements = parser->parse();
    if (diagnosticEngine.encounteredErrors()) {
        return nullptr;
    }

    auto buffer = std::make_unique<BytecodeBuffer>();
    for (const auto& statement : statements) {
        compileStatement(statement.get(), buffer.get());
    }

    buffer->update(static_cast<std::uint8_t>(InstructionType::OP_RETURN), 0);
    return buffer;
}

void BytecodeGenerator::compileStatement(Statement* statement, BytecodeBuffer* buffer) {
    if (auto* expressionStatement = dynamic_cast<ExpressionStatement*>(statement)) {
        compileExpressionStatement(expressionStatement, buffer);
    } else if (auto* variableStatement = dynamic_cast<VariableStatement*>(statement)) {
        compileVariableStatement(variableStatement, buffer);
    }
}

void BytecodeGenerator::compileExpressionStatement(ExpressionStatement* statement, BytecodeBuffer* buffer) {
    compileExpression(statement->expression.get(), buffer);
    buffer->update(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
}

void BytecodeGenerator::compileVariableStatement(VariableStatement* statement, BytecodeBuffer* buffer) {
    if (statement->initializer) {
        compileExpression(statement->initializer.get(), buffer);
    } else {
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NIL), statement->name.sourceLocation.line);
    }

    buffer->values.emplace_back(statement->name.lexeme);
    buffer->update(static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL), statement->name.sourceLocation.line);
    buffer->update(static_cast<std::uint8_t>(buffer->values.size() - 1), statement->name.sourceLocation.line);
}

void BytecodeGenerator::compileExpression(Expression* originalExpression, BytecodeBuffer* buffer) {
    if (auto* variableExpression = dynamic_cast<VariableExpression*>(originalExpression)) {
        compileVariableExpression(variableExpression, buffer);
    } else if (const auto* groupingExpression = dynamic_cast<GroupingExpression*>(originalExpression)) {
        compileGroupingExpression(groupingExpression, buffer);
    } else if (const auto* binaryExpression = dynamic_cast<BinaryExpression*>(originalExpression)) {
        compileBinaryExpression(binaryExpression, buffer);
    } else if (const auto* unaryExpression = dynamic_cast<UnaryExpression*>(originalExpression)) {
        compileUnaryExpression(unaryExpression, buffer);
    } else if (const auto* literalExpression = dynamic_cast<LiteralExpression*>(originalExpression)) {
        compileLiteralExpression(literalExpression, buffer);
    }
}

void BytecodeGenerator::compileVariableExpression(VariableExpression* originalExpression, BytecodeBuffer* buffer) {
    buffer->values.emplace_back(originalExpression->name.lexeme);
    buffer->update(static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL), originalExpression->name.sourceLocation.line);
    buffer->update(static_cast<std::uint8_t>(buffer->values.size() - 1), originalExpression->name.sourceLocation.line);
}

void BytecodeGenerator::compileGroupingExpression(const GroupingExpression* originalExpression, BytecodeBuffer* buffer) {
    compileExpression(originalExpression->expression.get(), buffer);
}

void BytecodeGenerator::compileBinaryExpression(const BinaryExpression* originalExpression, BytecodeBuffer* buffer) {
        compileExpression(originalExpression->lhs.get(), buffer);
        compileExpression(originalExpression->rhs.get(), buffer);

        const auto line = originalExpression->binaryOperator.sourceLocation.line;
        switch (originalExpression->binaryOperator.kind) {
            case TokenKind::PLUS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_ADD), line);
                break;
            }
            case TokenKind::MINUS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_MINUS), line);
                break;
            }
            case TokenKind::STAR: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_STAR), line);
                break;
            }
            case TokenKind::SLASH: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_SLASH), line);
                break;
            }
            case TokenKind::MODULO: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_MODULO), line);
                break;
            }
            case TokenKind::POWER: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_POWER), line);
                break;
            }
            case TokenKind::BITWISE_AND: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND), line);
                break;
            }
            case TokenKind::BITWISE_OR: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR), line);
                break;
            }
            case TokenKind::BITWISE_XOR: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR), line);
                break;
            }
            case TokenKind::BITWISE_LEFT_SHIFT: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT), line);
                break;
            }
            case TokenKind::BITWISE_RIGHT_SHIFT: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT), line);
                break;
            }
            case TokenKind::EQUALS_EQUALS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS), line);
                break;
            }
            case TokenKind::GREATER: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_GREATER), line);
                break;
            }
            case TokenKind::GREATER_EQUALS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_GREATER_EQUALS), line);
                break;
            }
            case TokenKind::LESS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_LESS), line);
                break;
            }
            case TokenKind::LESS_EQUALS: {
                buffer->update(static_cast<std::uint8_t>(InstructionType::OP_LESS_EQUALS), line);
                break;
            }
            default:
                break;
        }
}

void BytecodeGenerator::compileUnaryExpression(const UnaryExpression* originalExpression, BytecodeBuffer* buffer) {
    compileExpression(originalExpression->expression.get(), buffer);

    const auto line = originalExpression->unaryOperator.sourceLocation.line;
    if (originalExpression->unaryOperator.kind == TokenKind::BANG) {
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NOT), line);
    } else if (originalExpression->unaryOperator.kind == TokenKind::MINUS) {
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NEGATE), line);
    } else if (originalExpression->unaryOperator.kind == TokenKind::BITWISE_NOT) {
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT), line);
    }
}

void BytecodeGenerator::compileLiteralExpression(const LiteralExpression* originalExpression, BytecodeBuffer* buffer) {
    buffer->insert(originalExpression->value, 1);
}
