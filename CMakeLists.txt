cmake_minimum_required(VERSION 4.2)
project(apex_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(apex_interpreter main.cpp
        include/Driver/Driver.h
        utils/common/Common.h
        src/Driver/Driver.cpp
        include/structures/Token/Token.h
        include/frontend/Scanner/Scanner.h
        include/structures/Token/TokenKind.h
        src/frontend/Scanner/Scanner.cpp
        utils/SourceLocation/SourceLocation.h
        include/structures/Diagnostics/LexerError/LexerError.h
        include/structures/Diagnostics/LexerError/LexerErrorCode.h
        include/structures/AST/Expr/Expression.h
        include/structures/AST/Expr/ExpressionVisitor.h
        include/frontend/Parser/Parser.h
        src/frontend/Parser/Parser.cpp
        include/diagnostics/DiagnosticEngine.h
        src/diagnostics/DiagnosticEngine.cpp
        include/structures/Diagnostic/Diagnostic.h
)

target_include_directories(apex_interpreter PRIVATE utils)
target_include_directories(apex_interpreter PRIVATE include)