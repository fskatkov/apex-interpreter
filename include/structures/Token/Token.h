#pragma once

#include "Common/Common.h"
#include "TokenKind.h"

struct Token {
    TokenKind kind;
    std::string lexeme;
    std::string literal;
    int line;
};
