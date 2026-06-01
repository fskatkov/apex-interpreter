cmake_minimum_required(VERSION 4.2)
project(apex_interpreter)

set(CMAKE_CXX_STANDARD 20)

add_executable(apex_interpreter main.cpp
        include/engine/Driver/Driver.h
        utils/common/Common.h
        src/engine/Driver/Driver.cpp
        include/structures/Token/Token.h
        include/engine/Scanner/Scanner.h
        include/structures/Token/TokenKind.h
        src/engine/Scanner/Scanner.cpp
        utils/Diagnostics/SourceLocation.h
        include/structures/Diagnostics/LexerError/LexerError.h
        include/structures/Diagnostics/LexerError/LexerErrorCode.h
        include/structures/AST/Expression.h
        include/structures/AST/BinaryExpression.h
        include/structures/AST/GroupingExpression.h
        include/structures/AST/LiteralExpression.h
        include/structures/AST/UnaryExpression.h
        include/structures/AST/ExpressionVisitor.h
        include/engine/Parser/Parser.h
        src/engine/Parser/Parser.cpp
)

target_include_directories(apex_interpreter PRIVATE utils)
target_include_directories(apex_interpreter PRIVATE include)