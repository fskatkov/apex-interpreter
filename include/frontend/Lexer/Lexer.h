#pragma once

#include "Common/Common.h"
#include "structures/Token/Token.h"
#include "diagnostics/DiagnosticEngine.h"

enum class LexerStringScanningMode {
    REGULAR_STRING,
    INTERPOLATED_STRING_TEXT,
    INTERPOLATED_STRING_EXPRESSION,
};

class Lexer {
public:
    explicit Lexer(std::string source, DiagnosticEngine& diagnostic_engine);
    std::vector<Token> scan();
    [[nodiscard]] const std::vector<Token>& get_tokens() const;
    [[nodiscard]] bool encountered_errors() const;
private:
    DiagnosticEngine& diagnostic_engine;

    std::string source;

    std::size_t start_position{0};
    std::size_t current_position{0};
    std::size_t line{1};
    std::size_t start_line{1};
    std::size_t column{1};
    std::size_t start_column{1};

    std::vector<Token> tokens;
    bool encountered_error = false;

    std::vector<LexerStringScanningMode> modes;
    int brace_depth{0};

    void scan_token();
    [[nodiscard]] bool is_reached_end() const;
    char advance();
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peek_next() const;
    bool match(const char& expected);
    [[nodiscard]] TokenKind check(std::size_t starting, std::size_t ending, const std::string_view &rest, TokenKind kind) const;
    void add(const TokenKind& kind, const Value& literal = std::make_shared<std::string>(""));
    void make_string_token();
    void make_interpolated_string_token();
    void make_number_token();
    void make_character_token();
    void make_identifier_token();
    [[nodiscard]] TokenKind check_identifier() const;
    void report_error(const std::string_view& message) const;
};
