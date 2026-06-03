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
    if (const auto* expressionStatement = dynamic_cast<ExpressionStatement*>(statement)) {
        compileExpression(expressionStatement->expression.get(), buffer);
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
    } else if (const auto* variableStatement = dynamic_cast<VariableStatement*>(statement)) {
        if (variableStatement->initializer) {
            compileExpression(variableStatement->initializer.get(), buffer);
        } else {
            buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NIL), variableStatement->name.sourceLocation.line);
        }

        buffer->values.emplace_back(variableStatement->name.lexeme);
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL), variableStatement->name.sourceLocation.line);
        buffer->update(static_cast<std::uint8_t>(buffer->values.size() - 1), variableStatement->name.sourceLocation.line);
    }
}

void BytecodeGenerator::compileExpression(Expression* originalExpression, BytecodeBuffer* buffer) {
    if (const auto* literalExpression = dynamic_cast<LiteralExpression*>(originalExpression)) {
        buffer->insert(literalExpression->value, 1);
    } else if (const auto* binaryExpression = dynamic_cast<BinaryExpression*>(originalExpression)) {
        compileExpression(binaryExpression->lhs.get(), buffer);
        compileExpression(binaryExpression->rhs.get(), buffer);

        const auto line = binaryExpression->binaryOperator.sourceLocation.line;
        switch (binaryExpression->binaryOperator.kind) {
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
    } else if (const auto* groupingExpression = dynamic_cast<GroupingExpression*>(originalExpression)) {
        compileExpression(groupingExpression->expression.get(), buffer);
    } else if (const auto* unaryExpression = dynamic_cast<UnaryExpression*>(originalExpression)) {
        compileExpression(unaryExpression->expression.get(), buffer);

        const auto line = unaryExpression->unaryOperator.sourceLocation.line;
        if (unaryExpression->unaryOperator.kind == TokenKind::BANG) {
            buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NOT), line);
        } else if (unaryExpression->unaryOperator.kind == TokenKind::MINUS) {
            buffer->update(static_cast<std::uint8_t>(InstructionType::OP_NEGATE), line);
        } else if (unaryExpression->unaryOperator.kind == TokenKind::BITWISE_NOT) {
            buffer->update(static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT), line);
        }
    } else if (const auto* variableExpression = dynamic_cast<VariableExpression*>(originalExpression)) {
        buffer->values.emplace_back(variableExpression->name.lexeme);
        buffer->update(static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL), variableExpression->name.sourceLocation.line);
        buffer->update(static_cast<std::uint8_t>(buffer->values.size() - 1), variableExpression->name.sourceLocation.line);
    }
}