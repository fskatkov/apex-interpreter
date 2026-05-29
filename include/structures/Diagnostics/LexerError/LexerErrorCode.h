#pragma once

enum class LexerErrorCode {
    UnexpectedCharacter,
    UnterminatedString,
    InvalidNumberFormat,
    UnknownToken
};
