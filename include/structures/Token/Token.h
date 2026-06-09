#pragma once

#include "Common/Common.h"
#include "TokenKind.h"
#include "structures/Value/Value.h"
#include "../utils/SourceLocation/SourceLocation.h"

struct Token {
    TokenKind kind;
    std::string lexeme;
    Value literal;
    SourceLocation sourceLocation;
};
