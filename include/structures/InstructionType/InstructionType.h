#pragma once

enum class InstructionType {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_ADD,
    OP_MINUS,
    OP_STAR,
    OP_SLASH,

    OP_NOT,
    OP_NEGATE,

    OP_RETURN
};
