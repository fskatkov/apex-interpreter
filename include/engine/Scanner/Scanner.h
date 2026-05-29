#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "structures/Diagnostics/LexerError/LexerError.h"

class Scanner {
public:
    explicit Scanner(std::string& source);
    ~Scanner() = default;
    std::vector<Token> scan();
    std::vector<Token> tokens;
private:
    std::string source;
    std::size_t startPosition;
    std::size_t currentPosition;
    std::size_t line;
    std::size_t column;
    std::vector<LexerError> errors;

    void scanToken();
    bool isReachedEnd() const;
    char advance();
    char peek();
    char peekNext();
    bool match(const char& expected);
    TokenKind check(std::size_t starting, std::size_t ending, std::string rest, TokenKind kind);
    void add(const TokenKind& kind, const std::string& literal = "");
    void makeString();
    void makeNumber();
    void makeCharacter();
    void makeIdentifier();
    TokenKind checkIdentifierType();
    void raiseError(LexerErrorCode errorCode, std::string message);
};
