#pragma once

#include "Common/Common.h"
#include "frontend/Lexer/Lexer.h"
#include "frontend/Parser/Parser.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include "structures/BytecodeBuffer/BytecodeBuffer.h"

class BytecodeGenerator {
public:
    explicit BytecodeGenerator(DiagnosticEngine& diagnosticEngine);
    std::unique_ptr<BytecodeBuffer> generate(std::string& source);
private:
    DiagnosticEngine& diagnosticEngine;
    std::shared_ptr<Lexer> lexer;
    std::shared_ptr<Parser> parser;
    std::vector<Token> tokens;
    std::unique_ptr<Expression> expression;

    void compileExpression(Expression* originalExpression, BytecodeBuffer* buffer);
};
