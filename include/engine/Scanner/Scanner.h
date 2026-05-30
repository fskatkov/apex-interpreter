#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/Diagnostics/LexerError/LexerError.h"

class Scanner {
public:
    explicit Scanner(std::string& source);
    ~Scanner() = default;
    std::vector<Token> scan();
    const std::vector<Token>& getTokens() const;
    bool encounteredErrors() const;
    void raiseErrors() const;
private:
    std::string source;
    std::size_t startPosition;
    std::size_t currentPosition;
    std::size_t line;
    std::size_t startLine;
    std::size_t column;
    std::size_t startColumn;
    std::vector<Token> tokens;
    bool encounteredError;
    std::vector<LexerError> errors;

    void scanToken();
    bool isReachedEnd() const;
    char advance();
    char peek();
    char peekNext();
    bool match(const char& expected);
    TokenKind check(std::size_t starting, std::size_t ending, std::string rest, TokenKind kind);
    void add(const TokenKind& kind, const std::string& literal = "");
    void addStringToken();
    void addNumberToken();
    void addCharacterToken();
    void addIdentifierToken();
    TokenKind checkIdentifierType();
    void insertError(LexerErrorCode errorCode, std::string message);
    void raiseError(const LexerError& error) const;
};
