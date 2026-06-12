cmake_minimum_required(VERSION 4.2)
project(apex_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(apex_interpreter main.cpp
        include/REPL/REPL.h
        utils/common/Common.h
        src/REPL/REPL.cpp
        include/structures/Token/Token.h
        include/frontend/Lexer/Lexer.h
        include/structures/Token/TokenKind.h
        src/frontend/Lexer/Lexer.cpp
        utils/SourceLocation/SourceLocation.h
        include/structures/AST/Expr/Expr.h
        include/frontend/Parser/Parser.h
        src/frontend/Parser/Parser.cpp
        include/diagnostics/DiagnosticEngine.h
        src/diagnostics/DiagnosticEngine.cpp
        include/structures/Diagnostic/Diagnostic.h
        include/structures/BytecodeBuffer/BytecodeBuffer.h
        src/structures/BytecodeBuffer/BytecodeBuffer.cpp
        include/structures/InstructionType/InstructionType.h
        include/backend/ExecutionEngine/ExecutionEngine.h
        src/backend/ExecutionEngine/ExecutionEngine.cpp
        include/backend/BytecodeGenerator/BytecodeGenerator.h
        src/backend/BytecodeGenerator/BytecodeGenerator.cpp
        include/structures/AST/Stmt/Stmt.h
        include/structures/Value/Value.h
)

target_include_directories(apex_interpreter PRIVATE utils)
target_include_directories(apex_interpreter PRIVATE include)