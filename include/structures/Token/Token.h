#pragma once

#include "Common/Common.h"
#include "TokenKind.h"
#include "../utils/SourceLocation/SourceLocation.h"

struct Token {
    TokenKind kind;
    std::string lexeme;
    std::any literal;
    SourceLocation sourceLocation;
};
