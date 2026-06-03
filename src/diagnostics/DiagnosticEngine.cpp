#include "diagnostics/DiagnosticEngine.h"

DiagnosticEngine::DiagnosticEngine(const std::string& source, const std::string& REPL_TYPE)
    : REPL_TYPE(REPL_TYPE), source(source), hasErrors(false) {  }

void DiagnosticEngine::report(Diagnostic::DiagnosticKind kind, SourceLocation location, const std::string& message) {
    diagnostics.emplace_back(kind, location, message);

    if (kind == Diagnostic::DiagnosticKind::Error || kind == Diagnostic::DiagnosticKind::Fatal) {
        hasErrors = true;
    }
}

void DiagnosticEngine::raise() const {
    std::cout << "\033[31m";

    for (const auto& diagnostic : diagnostics) {
        std::cout << REPL_TYPE << diagnostic.location.line << ":" << diagnostic.location.column << ": ";
        if (diagnostic.kind == Diagnostic::DiagnosticKind::Warning) {
            std::cout << "warning: ";
        } else if (diagnostic.kind == Diagnostic::DiagnosticKind::Fatal) {
            std::cout << "runtime error: ";
        } else {
            std::cout << "compile-time error: ";
        }
        std::cout << diagnostic.message << "\n";

        std::stringstream stream(source);
        std::string codeBlock;
        for (int i = 0; i < diagnostic.location.line; ++i) {
            std::getline(stream, codeBlock);
        }

        std::cout << " " << diagnostic.location.line << " | " << codeBlock << "\n";

        const std::string padding(std::to_string(diagnostic.location.line).length(), ' ');
        std::cout << " " << padding << " | ";

        for (int i = 1; i < diagnostic.location.column; ++i) {
            std::cout << " ";
        }
        std::cout << "^";

        const auto errorLength = diagnostic.location.length > 0 ? diagnostic.location.length : 1;
        for (int i = 1; i < errorLength; ++i) {
            std::cout << "~";
        }
        std::cout << "\033[0m" << "\n";
    }
}

bool DiagnosticEngine::encounteredErrors() const {
    return hasErrors;
}