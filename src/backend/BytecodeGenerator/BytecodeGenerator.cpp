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
        }
    }
}