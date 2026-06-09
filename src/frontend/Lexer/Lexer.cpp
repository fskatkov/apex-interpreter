#include "frontend/Lexer/Lexer.h"

Lexer::Lexer(std::string& source, DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), source(std::move(source)), startPosition(0), currentPosition(0),
      line(1), startLine(1), column(1), startColumn(1), encounteredError(false) {  }

std::vector<Token> Lexer::scan() {
    while (!isReachedEnd()) {
        startPosition = currentPosition;
        startLine = line;
        startColumn = column;
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

const std::vector<Token>& Lexer::getTokens() const {
    return tokens;
}

bool Lexer::encounteredErrors() const {
    return encounteredError;
}

void Lexer::scanToken() {
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
            if (match('=')) {
                add(TokenKind::PLUS_EQUALS);
            } else if (match('+')) {
                add(TokenKind::INCREMENT);
            } else {
                add(TokenKind::PLUS);
            }

            break;
        }
        case '-': {
            if (match('=')) {
                add(TokenKind::MINUS_EQUALS);
            } else if (match('-')) {
                add(TokenKind::DECREMENT);
            } else {
                add(TokenKind::MINUS);
            }

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
            while (peek() != '\n' && !isReachedEnd()) {
                advance();
            }

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
        case ',': {
            add(TokenKind::COMMA);
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
        case '\n':
            break;
        case '"': {
            addStringToken();
            break;
        }
        case '\'':
            addCharacterToken();
            break;
        default: {
            if (std::isdigit(static_cast<unsigned char>(c))) {
                addNumberToken();
            } else if (std::isalpha(static_cast<unsigned char>(c))) {
                addIdentifierToken();
            } else {
                reportError("unexpected character");
                encounteredError = true;
            }

            break;
        }
    }
}

bool Lexer::isReachedEnd() const {
    return currentPosition >= source.length();
}

char Lexer::advance() {
    const auto symbol = source[currentPosition++];
    if (symbol == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return symbol;
}

char Lexer::peek() const {
    if (isReachedEnd())
        return '\0';
    return source[currentPosition];
}

char Lexer::peekNext() const {
    if (currentPosition + 1 >= source.length())
        return '\0';
    return source[currentPosition + 1];
}

bool Lexer::match(const char& expected) {
    if (isReachedEnd() || source[currentPosition] != expected)
        return false;
    currentPosition++;
    column++;
    return true;
}

TokenKind Lexer::check(std::size_t starting, std::size_t ending, const std::string& rest, TokenKind kind) const {
    if (currentPosition - startPosition == starting + ending) {
        if (const std::string_view text(source.data() + startPosition + starting, ending); text == rest) {
            return kind;
        }
    }

    return TokenKind::IDENTIFIER;
}

void Lexer::add(const TokenKind& kind, const std::any& literal) {
    const auto length = currentPosition - startPosition;

    tokens.emplace_back(
        kind,
        source.substr(startPosition, length),
        literal,
        SourceLocation{ startLine, startColumn, startPosition, length }
    );
}

void Lexer::addStringToken() {
    while (peek() != '"' && !isReachedEnd()) {
        advance();
    }

    if (isReachedEnd()) {
        reportError("unterminated string");
        encounteredError = true;
        return;
    }

    advance();
    add(TokenKind::STRING, source.substr(startPosition, currentPosition - startPosition));
}

void Lexer::addNumberToken() {
    while (std::isdigit(static_cast<unsigned char>(peek())))
        advance();
    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peekNext()))) {
        advance();
        while (std::isdigit(static_cast<unsigned char>(peek())))
            advance();
    }

    add(TokenKind::NUMBER, std::stod(source.substr(startPosition, currentPosition - startPosition)));
}

void Lexer::addCharacterToken() {
    advance();

    if (peek() == '\'') {
        reportError("empty character literal");
        encounteredError = true;
        advance();
        return;
    }

    if (isReachedEnd()) {
        reportError("unterminated character");
        encounteredError = true;
        return;
    }

    if (const auto symbol = advance(); symbol == '\\') {
        if (isReachedEnd()) {
            reportError("unterminated character");
            encounteredError = true;
            return;
        }
        advance();
    }

    if (isReachedEnd() || peek() != '\'') {
        reportError("unterminated character");
        encounteredError = true;
        return;
    }

    advance();
    add(TokenKind::CHARACTER, source.substr(startPosition + 1, currentPosition - startPosition - 2));
}

void Lexer::addIdentifierToken() {
    while (std::isalnum(static_cast<unsigned char>(peek()))) {
        advance();
    }

    add(checkIdentifierType(), "");
}

TokenKind Lexer::checkIdentifierType() const {
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
                    case 'o': {
                        return check(2, 1, "r", TokenKind::FOR);
                    }
                    case 'u': {
                        return check(2, 2, "nc", TokenKind::FUNCTION);
                    }
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'p': {
            return check(1, 4, "rint", TokenKind::PRINT);
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
                    default: {
                        return TokenKind::IDENTIFIER;
                    }
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'T': {
            return check(1, 3, "rue", TokenKind::TRUE);
        }
        case 'F': {
            return check(1, 4, "alse", TokenKind::FALSE);
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

void Lexer::reportError(const std::string& message) const {
    SourceLocation sourceLocation{
        startLine,
        startColumn,
        startPosition,
        currentPosition - startPosition
    };
    diagnosticEngine.report(Diagnostic::DiagnosticKind::Error, sourceLocation, message);
}