#pragma once

enum class LexerErrorCode {
    UnexpectedCharacter,
    UnterminatedString,
    UnterminatedCharacter,
    InvalidNumberFormat,
    UnknownToken
};
