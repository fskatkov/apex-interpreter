#include "engine/Scanner/Scanner.h"

Scanner::Scanner(std::string& source)
    : source(std::move(source)), startPosition(0), currentPosition(0), line(1), column(1) {  }

std::vector<Token> Scanner::scan() {
    while (!isReachedEnd()) {
        startPosition = currentPosition;
        scanToken();
    }

    tokens.emplace_back(
        TokenKind::END_OF_FILE,
        "",
        "",
        SourceLocation{ line, column, currentPosition, 0 }
    );
    return tokens;
}

void Scanner::scanToken() {
    switch (const auto c = advance(); c) {
        case '(': {
            add(TokenKind::LEFT_PAREN);
            break;
        }
        case ')': {
            add(TokenKind::RIGHT_PAREN);
            break;
        }
        case '{': {
            add(TokenKind::LEFT_BRACE);
            break;
        }
        case '}': {
            add(TokenKind::RIGHT_BRACE);
            break;
        }
        case '[': {
            add(TokenKind::LEFT_BRACKET);
            break;
        }
        case ']': {
            add(TokenKind::RIGHT_BRACKET);
            break;
        }
        case '+': {
            add(match('=') ? TokenKind::PLUS_EQUALS : TokenKind::PLUS);
            break;
        }
        case '-': {
            add(match('=') ? TokenKind::MINUS_EQUALS : TokenKind::MINUS);
            break;
        }
        case '*': {
            if (match('=')) {
                add(TokenKind::STAR_EQUALS);
            } else if (match('*')) {
                add(TokenKind::POWER);
            } else {
                add(TokenKind::STAR);
            }

            break;
        }
        case '/': {
            add(match('=') ? TokenKind::SLASH_EQUALS : TokenKind::SLASH);
            break;
        }
        case '%': {
            add(match('=') ? TokenKind::MODULO_EQUALS : TokenKind::MODULO);
            break;
        }
        case '&': {
            add(match('=') ? TokenKind::BITWISE_AND_EQUALS : TokenKind::BITWISE_AND);
            break;
        }
        case '|': {
            add(match('=') ? TokenKind::BITWISE_OR_EQUALS : TokenKind::BITWISE_OR);
            break;
        }
        case '^': {
            add(match('=') ? TokenKind::BITWISE_XOR_EQUALS : TokenKind::BITWISE_XOR);
            break;
        }
        case '~': {
            add(match('=') ? TokenKind::BITWISE_NOT_EQUALS : TokenKind::BITWISE_NOT);
            break;
        }
        case '#': {
            while (peek() != '\n' && !isReachedEnd()) advance();
            break;
        }
        case '!': {
            add(match('=') ? TokenKind::BANG_EQUALS : TokenKind::BANG);
            break;
        }
        case ';': {
            add(TokenKind::SEMICOLON);
            break;
        }
        case ':': {
            add(TokenKind::COLON);
            break;
        }
        case '.': {
            add(TokenKind::DOT);
            break;
        }
        case '=':
            add(match('=') ? TokenKind::EQUALS_EQUALS : TokenKind::EQUALS);
            break;
        case '<':
            if (match('=')) {
                add(TokenKind::LESS_EQUALS);
            } else if (match('<')) {
                if (match('=')) {
                    add(TokenKind::BITWISE_LEFT_SHIFT_EQUALS);
                } else {
                    add(TokenKind::BITWISE_LEFT_SHIFT);
                }
            } else {
                add(TokenKind::LESS);
            }

            break;
        case '>':
            if (match('=')) {
                add(TokenKind::GREATER_EQUALS);
            } else if (match('>')) {
                if (match('=')) {
                    add(TokenKind::BITWISE_RIGHT_SHIFT_EQUALS);
                } else {
                    add(TokenKind::BITWISE_RIGHT_SHIFT);
                }
            } else {
                add(TokenKind::GREATER);
            }

            break;
        case ' ':
        case '\r':
        case '\t':
            break;
        case '\n':
            line++;
            column = 1;
            break;
        case '"': {
            addStringToken();
            break;
        }
        case '\'':
            addCharacterToken();
            break;
        default: {
            if (std::isdigit(c)) {
                addNumberToken();
            } else if (std::isalpha(c)) {
                addIdentifierToken();
            } else {
                insertError(LexerErrorCode::UnexpectedCharacter, "unexpected character");
                encounteredError = true;
            }

            break;
        }
    }
}

bool Scanner::isReachedEnd() const {
    return currentPosition >= source.length();
}

char Scanner::advance() {
    const auto c = source[currentPosition];
    currentPosition++;
    column++;
    return c;
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

TokenKind Scanner::check(std::size_t starting, std::size_t ending, std::string rest, TokenKind kind) {
    if (currentPosition - startPosition == starting + ending && source.substr(startPosition + starting, ending) == rest)
        return kind;
    return TokenKind::IDENTIFIER;
}

void Scanner::add(const TokenKind& kind, const std::string& literal) {
    const auto length = currentPosition - startPosition;
    const auto startColumn = column - length;

    tokens.emplace_back(
        kind,
        source.substr(startPosition, length),
        literal,
        SourceLocation{ line, startColumn, startPosition, length }
    );
}

void Scanner::addStringToken() {
    while (peek() != '"' && !isReachedEnd()) {
        if (peek() == '\n') {
            line++;
            column = 1;
        }
        advance();
    }

    if (isReachedEnd()) {
        insertError(LexerErrorCode::UnterminatedString, "unterminated string");
        encounteredError = true;
        return;
    }

    advance();
    add(TokenKind::STRING, source.substr(startPosition + 1, currentPosition - startPosition - 2));
}

void Scanner::addNumberToken() {
    while (std::isdigit(peek()))
        advance();
    if (peek() == '.' && std::isdigit(peekNext())) {
        advance();
        while (std::isdigit(peek()))
            advance();
    }

    add(TokenKind::NUMBER, source.substr(startPosition, currentPosition - startPosition));
}

void Scanner::addCharacterToken() {
    advance();
    if (isReachedEnd()) {
        insertError(LexerErrorCode::UnterminatedCharacter, "unterminated character");
        return;
    }
    advance();
    add(TokenKind::CHARACTER, source.substr(startPosition, currentPosition - startPosition));
}

void Scanner::addIdentifierToken() {
    while (std::isalnum(peek())) {
        advance();
    }

    add(checkIdentifierType(), "");
}

TokenKind Scanner::checkIdentifierType() {
    switch (source[startPosition]) {
        case 'a': {
            return check(1, 2, "nd", TokenKind::AND);
        }
        case 'c': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'l': {
                        return check(2, 3, "ass", TokenKind::CLASS);
                    }
                    case 'o': {
                        if (currentPosition - startPosition > 2 && source[startPosition + 2] == 'n') {
                            if (currentPosition - startPosition > 3) {
                                switch (source[startPosition + 3]) {
                                    case 's': {
                                        return check(4, 1, "t", TokenKind::CONST);
                                    }
                                    case 't': {
                                        return check(4, 4, "inue", TokenKind::CONTINUE);
                                    }
                                    default: {
                                        return TokenKind::IDENTIFIER;
                                    }
                                }
                            }
                        }

                        break;
                    }
                    case 'a': {
                        return check(2, 2, "se", TokenKind::CASE);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'e': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'l': {
                        return check(2, 2, "se", TokenKind::ELSE);
                    }
                    case 'n': {
                        return check(2, 2, "um", TokenKind::ENUM);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'f': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'a': {
                        return check(2, 3, "lse", TokenKind::FALSE);
                    }
                    case 'o': {
                        return check(2, 1, "r", TokenKind::FOR);
                    }
                    case 'u': {
                        return check(2, 6, "nction", TokenKind::FUNCTION);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'i': {
            return check(1, 1, "f", TokenKind::IF);
        }
        case 'n': {
            return check(1, 3, "ull", TokenKind::NIL);
        }
        case 'o': {
            return check(1, 1, "r", TokenKind::OR);
        }
        case 'r': {
            return check(1, 5, "eturn", TokenKind::RETURN);
        }
        case 's': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'u': {
                        return check(2, 3, "per", TokenKind::SUPER);
                    }
                    case 'w': {
                        return check(2, 4, "itch", TokenKind::SWITCH);
                    }
                    case 't': {
                        return check(2, 4, "ruct", TokenKind::STRUCTURE);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 't': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'h': {
                        return check(2, 2, "is", TokenKind::THIS);
                    }
                    case 'r': {
                        return check(2, 2, "ue", TokenKind::TRUE);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'v': {
            return check(1, 2, "ar", TokenKind::VAR);
        }
        case 'w': {
            return check(1, 4, "hile", TokenKind::WHILE);
        }
        case 'd': {
            if (currentPosition - startPosition > 1) {
                switch (source[startPosition + 1]) {
                    case 'e': {
                        return check(2, 5, "fault", TokenKind::DEFAULT);
                    }
                    case 'o': {
                        return check(2, 0, "", TokenKind::DO);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'b': {
            return check(1, 4, "reak", TokenKind::BREAK);
        }
        default: {
            return TokenKind::IDENTIFIER;
        }
    }
}

void Scanner::insertError(LexerErrorCode errorCode, std::string message) {
    SourceLocation sourceLocation{
        line,
        column - (currentPosition - startPosition),
        startPosition,
        currentPosition - startPosition
    };

    errors.emplace_back(errorCode, sourceLocation, std::move(message));
}

void Scanner::raiseError(const LexerError& error) const {
    std::cerr << "current:" << error.sourceLocation.line << ":" << error.sourceLocation.column
    << ": SyntaxError" << error.message << "\n";
}

void Scanner::raiseErrors() const {
    if (encounteredError) {
        for (const auto& error : errors) {
            raiseError(error);
        }
    }
}
