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
    expression = parser->parse();
    if (!expression) {
        return nullptr;
    }

    auto buffer = std::make_unique<BytecodeBuffer>();
    compileExpression(expression.get(), buffer.get());
    buffer->update(static_cast<std::uint8_t>(InstructionType::OP_RETURN), 0);
    return buffer;
}

void BytecodeGenerator::compileExpression(Expression* originalExpression, BytecodeBuffer* buffer) {
    if (auto* literalExpression = dynamic_cast<LiteralExpression*>(originalExpression)) {
        buffer->insert(literalExpression->value, 1);
    } else if (auto* binaryExpression = dynamic_cast<BinaryExpression*>(originalExpression)) {
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
            default:
                break;
        }
    } else if (auto* groupingExpression = dynamic_cast<GroupingExpression*>(originalExpression)) {
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
    }
}