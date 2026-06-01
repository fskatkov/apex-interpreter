#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"

class Scanner {
public:
    explicit Scanner(std::string& source, DiagnosticEngine& diagnosticEngine);
    ~Scanner() = default;
    std::vector<Token> scan();
    [[nodiscard]] const std::vector<Token>& getTokens() const;
    [[nodiscard]] bool encounteredErrors() const;
private:
    DiagnosticEngine& diagnosticEngine;

    std::string source;
    std::size_t startPosition;
    std::size_t currentPosition;
    std::size_t line;
    std::size_t startLine;
    std::size_t column;
    std::size_t startColumn;
    std::vector<Token> tokens;
    bool encounteredError;

    void scanToken();
    [[nodiscard]] bool isReachedEnd() const;
    char advance();
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peekNext() const;
    bool match(const char& expected);
    [[nodiscard]] TokenKind check(std::size_t starting, std::size_t ending, const std::string &rest, TokenKind kind) const;
    void add(const TokenKind& kind, const std::any& literal = "");
    void addStringToken();
    void addNumberToken();
    void addCharacterToken();
    void addIdentifierToken();
    [[nodiscard]] TokenKind checkIdentifierType() const;
    void reportError(const std::string& message) const;
};
