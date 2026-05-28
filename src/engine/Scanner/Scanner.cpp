#include "engine/Scanner/Scanner.h"

Scanner::Scanner(std::string& source)
    : source(std::move(source)), startPosition(0), currentPosition(0), line(1) {  }

std::vector<Token> Scanner::scan() {
    while (!isReachedEnd()) {
        startPosition = currentPosition;
        scanToken();
    }

    tokens.emplace_back(TokenKind::END_OF_FILE, "", "", line);
    return tokens;
}

void Scanner::scanToken() {

}

bool Scanner::isReachedEnd() const {
    return currentPosition >= source.length();
}

char Scanner::advance() {
    return source[currentPosition++];
}

char Scanner::peek() {
    if (isReachedEnd())
        return '\0';
    return source[currentPosition];
}

char Scanner::peekNext() {
    if (currentPosition + 1 >= source.length())
        return '\0';
    return source[currentPosition + 1];
}

bool Scanner::match(const char& expected) {
    if (isReachedEnd() || source[currentPosition] != expected)
        return false;
    currentPosition++;
    return true;
}

void Scanner::add(const TokenKind& kind) {
    add(kind, "");
}

void Scanner::add(const TokenKind& kind, const std::string& literal) {
    tokens.emplace_back(kind, source.substr(startPosition, currentPosition - startPosition), literal, line);
}

void Scanner::makeString() {

}

void Scanner::makeNumber() {

}

void Scanner::makeCharacter() {

}

void Scanner::makeIdentifier() {

}
