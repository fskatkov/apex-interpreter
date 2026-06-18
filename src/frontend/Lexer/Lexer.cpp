#include "frontend/Lexer/Lexer.h"

Lexer::Lexer(std::string source, DiagnosticEngine &diagnostic_engine)
    : diagnostic_engine(diagnostic_engine), source(std::move(source)) {
}

std::vector<Token> Lexer::scan() {
    while (!is_reached_end()) {
        start_position = current_position;
        start_line = line;
        start_column = column;
        scan_token();
    }

    tokens.emplace_back(
        TokenKind::END_OF_FILE,
        "",
        "",
        SourceLocation{
            .line = line,
            .column = column,
            .offset = current_position,
            .length = 0
        }
    );

    return tokens;
}

const std::vector<Token> &Lexer::get_tokens() const {
    return tokens;
}

bool Lexer::encountered_errors() const {
    return encountered_error;
}

void Lexer::scan_token() {
    if (!modes.empty() && modes.back() == LexerStringScanningMode::INTERPOLATED_STRING_TEXT) [[unlikely]] {
        make_interpolated_string_token();
        return;
    }

    switch (const auto c = advance(); c) {
        case '(': add(TokenKind::LEFT_PAREN); break;
        case ')': add(TokenKind::RIGHT_PAREN); break;
        case '{': {
            add(TokenKind::LEFT_BRACE);

            if (!modes.empty() && modes.back() == LexerStringScanningMode::INTERPOLATED_STRING_EXPRESSION) {
                brace_depth++;
            }

            break;
        }
        case '}': {
            add(TokenKind::RIGHT_BRACE);

            if (!modes.empty() && modes.back() == LexerStringScanningMode::INTERPOLATED_STRING_EXPRESSION) {
                if (brace_depth > 1) {
                    brace_depth--;
                } else {
                    brace_depth = 0;
                    modes.pop_back();
                    modes.push_back(LexerStringScanningMode::INTERPOLATED_STRING_TEXT);
                }
            }

            break;
        }
        case '[': add(TokenKind::LEFT_BRACKET); break;
        case ']': add(TokenKind::RIGHT_BRACKET); break;
        case '+': {
            if (match('=')) add(TokenKind::PLUS_EQUALS);
            else if (match('+')) add(TokenKind::INCREMENT);
            else add(TokenKind::PLUS);
            break;
        }
        case '-': {
            if (match('=')) add(TokenKind::MINUS_EQUALS);
            else if (match('-')) add(TokenKind::DECREMENT);
            else add(TokenKind::MINUS);
            break;
        }
        case '*': {
            if (match('=')) add(TokenKind::STAR_EQUALS);
            else if (match('*')) add(TokenKind::POWER);
            else add(TokenKind::STAR);
            break;
        }
        case '/': add(match('=') ? TokenKind::SLASH_EQUALS : TokenKind::SLASH); break;
        case '%': add(match('=') ? TokenKind::MODULO_EQUALS : TokenKind::MODULO); break;
        case '&': add(match('=') ? TokenKind::AMPERSAND_EQUALS : TokenKind::AMPERSAND); break;
        case '|': add(match('=') ? TokenKind::PIPE_EQUALS : TokenKind::PIPE); break;
        case '^': add(match('=') ? TokenKind::CARET_EQUALS : TokenKind::CARET); break;
        case '~': add(match('=') ? TokenKind::TILDE_EQUALS : TokenKind::TILDE); break;
        case '#': {
            while (peek() != '\n' && !is_reached_end()) {
                advance();
            }
            break;
        }
        case '!': add(match('=') ? TokenKind::BANG_EQUALS : TokenKind::BANG); break;
        case '?': add(TokenKind::QUESTION_MARK); break;
        case ';': add(TokenKind::SEMICOLON); break;
        case ':': add(TokenKind::COLON); break;
        case '.': add(TokenKind::DOT); break;
        case ',': add(TokenKind::COMMA); break;
        case '=': add(match('=') ? TokenKind::EQUALS_EQUALS : TokenKind::EQUALS); break;
        case '<': {
            if (match('=')) add(TokenKind::LESS_EQUALS);
            else if (match('<')) add(match('=') ? TokenKind::LEFT_ANGLE_EQUALS : TokenKind::LEFT_ANGLE);
            else add(TokenKind::LESS);
            break;
        }
        case '>': {
            if (match('=')) add(TokenKind::GREATER_EQUALS);
            else if (match('>')) add(match('=') ? TokenKind::RIGHT_ANGLE_EQUALS : TokenKind::RIGHT_ANGLE);
            else add(TokenKind::GREATER);
            break;
        }
        case ' ':
        case '\r':
        case '\t':
        case '\n':
            break;
        case '\"': make_string_token(); break;
        case '\'': make_character_token(); break;
        default: {
            if (c == 'f' && peek() == '"') {
                advance();
                add(TokenKind::F_STRING_START);
                modes.push_back(LexerStringScanningMode::INTERPOLATED_STRING_TEXT);
                break;
            }

            if (std::isdigit(static_cast<unsigned char>(c))) {
                make_number_token();
            } else if (std::isalpha(static_cast<unsigned char>(c))) {
                make_identifier_token();
            } else [[unlikely]] {
                report_error("unexpected character");
                encountered_error = true;
            }
            break;
        }
    }
}

bool Lexer::is_reached_end() const {
    return current_position >= source.length();
}

char Lexer::advance() {
    const auto symbol = source[current_position++];
    if (symbol == '\n') [[unlikely]] {
        line++;
        column = 1;
    } else [[likely]] {
        column++;
    }
    return symbol;
}

char Lexer::peek() const {
    if (is_reached_end()) [[unlikely]] return '\0';
    return source[current_position];
}

char Lexer::peek_next() const {
    if (current_position + 1 >= source.length()) [[unlikely]] return '\0';
    return source[current_position + 1];
}

bool Lexer::match(const char &expected) {
    if (is_reached_end() || source[current_position] != expected) return false;
    current_position++;
    column++;
    return true;
}

TokenKind Lexer::check(const std::size_t starting, const std::size_t ending, const std::string_view &rest,
                       const TokenKind kind) const {
    if (current_position - start_position == starting + ending) {
        if (const std::string_view text(source.data() + start_position + starting, ending); text == rest) {
            return kind;
        }
    }

    return TokenKind::IDENTIFIER;
}

void Lexer::add(const TokenKind &kind, const Value &literal) {
    const auto length = current_position - start_position;

    tokens.emplace_back(
        kind,
        source.substr(start_position, length),
        literal,
        SourceLocation{
            .line = start_line,
            .column = start_column,
            .offset = start_position,
            .length = length
        }
    );
}

void Lexer::make_string_token() {
    while (peek() != '"' && !is_reached_end()) {
        advance();
    }

    if (is_reached_end()) [[unlikely]] {
        report_error("unterminated string");
        encountered_error = true;
        return;
    }

    advance();
    add(TokenKind::STRING, source.substr(start_position + 1, current_position - start_position - 2));
}

void Lexer::make_interpolated_string_token() {
    while (peek() != '{' && peek() != '"' && !is_reached_end()) {
        advance();
    }

    if (is_reached_end()) [[unlikely]] {
        report_error("unterminated string");
        encountered_error = true;
        return;
    }

    if (current_position > start_position) {
        add(TokenKind::F_STRING_SLICE, source.substr(start_position, current_position - start_position));
    }

    if (peek() == '{') {
        modes.pop_back();
        modes.push_back(LexerStringScanningMode::INTERPOLATED_STRING_EXPRESSION);
        brace_depth = 0;
    } else {
        advance();
        add(TokenKind::F_STRING_END);
        modes.pop_back();
    }
}

void Lexer::make_number_token() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
        advance();

        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    const std::string_view numericString(source.data() + start_position, current_position - start_position);
    double value = 0;

    const auto [ptr, result] =
            std::from_chars(numericString.data(), numericString.data() + numericString.size(), value);

    if (result == std::errc()) [[likely]] {
        add(TokenKind::NUMBER, value);
    } else [[unlikely]] {
        report_error("invalid number format");
        encountered_error = true;
    }
}

void Lexer::make_character_token() {
    if (peek() == '\'') [[unlikely]] {
        report_error("empty character literal");
        encountered_error = true;
        advance();
        return;
    }

    if (is_reached_end()) [[unlikely]] {
        report_error("unterminated character");
        encountered_error = true;
        return;
    }

    if (const auto symbol = advance(); symbol == '\\') {
        if (is_reached_end()) [[unlikely]] {
            report_error("unterminated character");
            encountered_error = true;
            return;
        }

        advance();
    }

    if (is_reached_end() || peek() != '\'') [[unlikely]] {
        report_error("unterminated character");
        encountered_error = true;
        return;
    }

    advance();
    add(TokenKind::CHARACTER, source[start_position + 1]);
}

void Lexer::make_identifier_token() {
    while (std::isalnum(static_cast<unsigned char>(peek()))) {
        advance();
    }

    add(check_identifier(), "");
}

TokenKind Lexer::check_identifier() const {
    switch (source[start_position]) {
        case 'a': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'n': return check(2, 1, "d", TokenKind::AND);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'c': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'l': return check(2, 3, "ass", TokenKind::CLASS);
                    case 'o': {
                        if (current_position - start_position > 2 && source[start_position + 2] == 'n') {
                            if (current_position - start_position > 3) {
                                switch (source[start_position + 3]) {
                                    case 's': return check(4, 1, "t", TokenKind::CONST);
                                    case 't': return check(4, 4, "inue", TokenKind::CONTINUE);
                                    default: return TokenKind::IDENTIFIER;
                                }
                            }
                        }

                        break;
                    }
                    case 'a': return check(2, 2, "se", TokenKind::CASE);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'e': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'l': return check(2, 2, "se", TokenKind::ELSE);
                    case 'n': return check(2, 2, "um", TokenKind::ENUM);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'f': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'o': return check(2, 1, "r", TokenKind::FOR);
                    case 'u': return check(2, 2, "nc", TokenKind::FUNCTION);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'i': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'f': return check(2, 0, "", TokenKind::IF);
                    case 'n': return check(2, 0, "", TokenKind::IN);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'n': return check(1, 3, "ull", TokenKind::NIL);
        case 'o': return check(1, 1, "r", TokenKind::OR);
        case 'r': return check(1, 5, "eturn", TokenKind::RETURN);
        case 's': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'u': return check(2, 3, "per", TokenKind::SUPER);
                    case 'w': return check(2, 4, "itch", TokenKind::SWITCH);
                    case 't': return check(2, 4, "ruct", TokenKind::STRUCTURE);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 't': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'h': return check(2, 2, "is", TokenKind::THIS);
                    case 'y': return check(2, 4, "peof", TokenKind::TYPEOF);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'T': return check(1, 3, "rue", TokenKind::TRUE);
        case 'F': return check(1, 4, "alse", TokenKind::FALSE);
        case 'v': return check(1, 2, "ar", TokenKind::VAR);
        case 'w': return check(1, 4, "hile", TokenKind::WHILE);
        case 'd': {
            if (current_position - start_position > 1) {
                switch (source[start_position + 1]) {
                    case 'e': return check(2, 5, "fault", TokenKind::DEFAULT);
                    case 'o': return check(2, 0, "", TokenKind::DO);
                    default: return TokenKind::IDENTIFIER;
                }
            }

            return TokenKind::IDENTIFIER;
        }
        case 'b': return check(1, 4, "reak", TokenKind::BREAK);
        default: return TokenKind::IDENTIFIER;
    }
}

void Lexer::report_error(const std::string_view& message) const {
    diagnostic_engine.report(
        Diagnostic::DiagnosticKind::Error,
        SourceLocation{
            .line = start_line,
            .column = start_column,
            .offset = start_position,
            .length = current_position - start_position
        },
        std::string(message)
    );
}
