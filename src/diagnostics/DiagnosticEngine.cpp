#include "diagnostics/DiagnosticEngine.h"

DiagnosticEngine::DiagnosticEngine(const std::string& source)
    : source(source), hasErrors(false) {  }

void DiagnosticEngine::report(Diagnostic::DiagnosticKind kind, SourceLocation location, const std::string& message) {
    diagnostics.emplace_back(kind, location, message);

    if (kind == Diagnostic::DiagnosticKind::Error || kind == Diagnostic::DiagnosticKind::Fatal) {
        hasErrors = true;
    }
}

void DiagnosticEngine::raise() const {
    for (const auto& diagnostic : diagnostics) {
        std::cerr << "<stdin>: " << diagnostic.location.line << ":" << diagnostic.location.column << ": ";
        if (diagnostic.kind == Diagnostic::DiagnosticKind::Warning) {
            std::cerr << "warning: ";
        } else if (diagnostic.kind == Diagnostic::DiagnosticKind::Fatal) {
            std::cerr << "fatal error: ";
        } else {
            std::cerr << "error: ";
        }
        std::cerr << diagnostic.message << "\n";

        std::stringstream stream(source);
        std::string codeBlock;
        for (int i = 0; i < diagnostic.location.line; ++i) {
            std::getline(stream, codeBlock);
        }

        std::cerr << " " << diagnostic.location.line << " | " << codeBlock << "\n";

        const std::string padding(std::to_string(diagnostic.location.line).length(), ' ');
        std::cerr << " " << padding << " | ";

        for (int i = 1; i < diagnostic.location.column; ++i) {
            std::cerr << " ";
        }
        std::cerr << "^";

        const auto errorLength = diagnostic.location.length > 0 ? diagnostic.location.length : 1;
        for (int i = 1; i < errorLength; ++i) {
            std::cerr << "~";
        }
        std::cerr << "\n\n";
    }
}

bool DiagnosticEngine::encounteredErrors() const {
    return hasErrors;
}