#pragma once

#include "Common/Common.h"
#include "Diagnostics/SourceLocation.h"
#include "LexerErrorCode.h"

struct LexerError {
    LexerErrorCode errorCode;
    SourceLocation sourceLocation;
    std::string message;
};
