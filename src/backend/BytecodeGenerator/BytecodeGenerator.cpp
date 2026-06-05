#include "backend/BytecodeGenerator/BytecodeGenerator.h"

BytecodeGenerator::BytecodeGenerator(DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), buffer(nullptr), lexer(nullptr), parser(nullptr) {  }

void BytecodeGenerator::generate(std::string& source) {
    lexer = std::make_shared<Lexer>(source, diagnosticEngine);
    tokens = lexer->scan();
    if (lexer->encounteredErrors()) return;

    parser = std::make_shared<Parser>(tokens, diagnosticEngine);
    statements = parser->parse();
    if (diagnosticEngine.encounteredErrors()) return;

    buffer = std::make_unique<BytecodeBuffer>();
    for (const auto& statement : statements) {
        compileStatement(statement.get());
    }

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_RETURN), 0);
}

void BytecodeGenerator::compileStatement(Statement* statement) {
    if (auto* expressionStatement = dynamic_cast<ExpressionStatement*>(statement)) {
        compileExpressionStatement(expressionStatement);
    } else if (auto* variableStatement = dynamic_cast<VariableStatement*>(statement)) {
        compileVariableStatement(variableStatement);
    } else if (auto* printStatement = dynamic_cast<PrintStatement*>(statement)) {
        compilePrintStatement(printStatement);
    }
}

void BytecodeGenerator::compilePrintStatement(const PrintStatement* statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_PRINT), 0);
}

void BytecodeGenerator::compileExpressionStatement(const ExpressionStatement* statement) {
    compileExpression(statement->expression.get());
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_POP), 0);
}

void BytecodeGenerator::compileVariableStatement(VariableStatement* statement) {
    if (statement->initializer) {
        compileExpression(statement->initializer.get());
    } else {
        emitByte(static_cast<std::uint8_t>(InstructionType::OP_NIL), statement->name.sourceLocation.line);
    }

    buffer->values.emplace_back(statement->name.lexeme);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL), statement->name.sourceLocation.line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), statement->name.sourceLocation.line);
}

void BytecodeGenerator::compileExpression(Expression* originalExpression) {
    if (auto* variableExpression = dynamic_cast<VariableExpression*>(originalExpression)) {
        compileVariableExpression(variableExpression);
    } else if (const auto* groupingExpression = dynamic_cast<GroupingExpression*>(originalExpression)) {
        compileGroupingExpression(groupingExpression);
    } else if (const auto* binaryExpression = dynamic_cast<BinaryExpression*>(originalExpression)) {
        compileBinaryExpression(binaryExpression);
    } else if (const auto* unaryExpression = dynamic_cast<UnaryExpression*>(originalExpression)) {
        compileUnaryExpression(unaryExpression);
    } else if (const auto* literalExpression = dynamic_cast<LiteralExpression*>(originalExpression)) {
        compileLiteralExpression(literalExpression);
    } else if (auto* assignmentExpression = dynamic_cast<AssignmentExpression*>(originalExpression)) {
        compileAssignmentExpression(assignmentExpression);
    }
}

void BytecodeGenerator::compileAssignmentExpression(AssignmentExpression* originalExpression) {
    compileExpression(originalExpression->value.get());

    buffer->values.emplace_back(originalExpression->name.lexeme);

    emitByte(static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL), originalExpression->name.sourceLocation.line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), originalExpression->name.sourceLocation.line);
}

void BytecodeGenerator::compileVariableExpression(VariableExpression* originalExpression) const {
    buffer->values.emplace_back(originalExpression->name.lexeme);
    emitByte(static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL), originalExpression->name.sourceLocation.line);
    emitByte(static_cast<std::uint8_t>(buffer->values.size() - 1), originalExpression->name.sourceLocation.line);
}

void BytecodeGenerator::compileGroupingExpression(const GroupingExpression* originalExpression) {
    compileExpression(originalExpression->expression.get());
}

void BytecodeGenerator::compileBinaryExpression(const BinaryExpression* originalExpression) {
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

void BytecodeGenerator::compileUnaryExpression(const UnaryExpression* originalExpression) {
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

void BytecodeGenerator::compileLiteralExpression(const LiteralExpression* originalExpression) const {
    buffer->insert(originalExpression->value, 1);
}

void BytecodeGenerator::emitByte(const std::uint8_t& byte, const std::size_t& line) const {
    buffer->update(byte, line);
}
