#pragma once

#include "Common/Common.h"
#include "../../structures/Token/Token.h"

class Scanner {
public:
    explicit Scanner(std::string& source);
    ~Scanner() = default;
    std::vector<Token> scan();
private:
    std::string source;
    std::size_t startPosition;
    std::size_t currentPosition;
    std::vector<Token> tokens;
    int line;

    void scanToken();
    bool isReachedEnd() const;
    char advance();
    char peek();
    char peekNext();
    bool match(const char& expected);
    TokenKind check(std::size_t starting, std::size_t ending, std::string rest, TokenKind kind);
    void add(const TokenKind& kind);
    void add(const TokenKind& kind, const std::string& literal);
    void makeString();
    void makeNumber();
    void makeCharacter();
    void makeIdentifier();
    TokenKind checkIdentifierType();

    std::unordered_map<std::string, TokenKind> keywords = {
        { "and", TokenKind::AND },
        { "class", TokenKind::CLASS },
        { "else", TokenKind::ELSE },
        { "False", TokenKind::FALSE },
        { "for", TokenKind::FOR },
        { "function", TokenKind::FUNC },
        { "if", TokenKind::IF },
        { "null", TokenKind::NIL },
        { "or", TokenKind::OR },
        { "return", TokenKind::RETURN },
        { "super", TokenKind::SUPER },
        { "this", TokenKind::THIS },
        { "True", TokenKind::TRUE },
        { "var", TokenKind::VAR },
        { "while", TokenKind::WHILE },
    };
};
