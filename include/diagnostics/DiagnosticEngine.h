#pragma once

#include "structures/Diagnostic/Diagnostic.h"

class DiagnosticEngine {
public:
    explicit DiagnosticEngine(const std::string& source, const std::string& REPL_TYPE = "<stdin>: ");
    void report(Diagnostic::DiagnosticKind kind, SourceLocation location, const std::string& message);
    void raise() const;
    [[nodiscard]] bool encounteredErrors() const;
private:
    std::vector<Diagnostic> diagnostics;
    std::string REPL_TYPE;
    std::string source;
    bool hasErrors;
};
