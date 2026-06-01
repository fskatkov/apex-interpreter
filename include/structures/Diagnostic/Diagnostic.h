#pragma once

#include "Common/Common.h"
#include "SourceLocation/SourceLocation.h"

struct Diagnostic {
    enum class DiagnosticKind {
        Note,
        Warning,
        Error,
        Fatal
    };

    DiagnosticKind kind;
    SourceLocation location;
    std::string message;
};
