#include "backend/ExecutionEngine/ExecutionEngine.h"

ExecutionEngine::ExecutionEngine(std::string &source, DiagnosticEngine &diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), source(source), buffer(nullptr), address(nullptr) {
    stack.reserve(256);
    registerStandardLibrary();
}

ExecutionResult ExecutionEngine::run() {
    BytecodeGenerator generator(diagnosticEngine);
    generator.generate(source);

    auto compiledBuffer = std::move(generator.buffer);
    if (!compiledBuffer || diagnosticEngine.encounteredErrors()) {
        diagnosticEngine.raise();
        return ExecutionResult::COMPILETIME_ERROR;
    }

    this->buffer = std::move(compiledBuffer);
    this->address = this->buffer->code.data();
    return execute();
}

const std::array<ExecutionEngine::Handler, 256> ExecutionEngine::dispatchTable = [] {
    std::array<ExecutionEngine::Handler, 256> table{};
    table.fill(&ExecutionEngine::executeUnknown);

    table[static_cast<std::uint8_t>(InstructionType::OP_CONSTANT)] = &ExecutionEngine::executeConstant;
    table[static_cast<std::uint8_t>(InstructionType::OP_TRUE)] = &ExecutionEngine::executeTrueLiteral;
    table[static_cast<std::uint8_t>(InstructionType::OP_FALSE)] = &ExecutionEngine::executeFalseLiteral;
    table[static_cast<std::uint8_t>(InstructionType::OP_NULL)] = &ExecutionEngine::executeNullLiteral;
    table[static_cast<std::uint8_t>(InstructionType::OP_BUILD_STRING)] = &ExecutionEngine::executeInterpolatedStringLiteral;

    table[static_cast<std::uint8_t>(InstructionType::OP_ADD)] = &ExecutionEngine::executeAddition;
    table[static_cast<std::uint8_t>(InstructionType::OP_SUB)] = &ExecutionEngine::executeSubtraction;
    table[static_cast<std::uint8_t>(InstructionType::OP_MUL)] = &ExecutionEngine::executeMultiplication;
    table[static_cast<std::uint8_t>(InstructionType::OP_DIV)] = &ExecutionEngine::executeDivision;
    table[static_cast<std::uint8_t>(InstructionType::OP_MOD)] = &ExecutionEngine::executeModuloDivision;
    table[static_cast<std::uint8_t>(InstructionType::OP_POW)] = &ExecutionEngine::executePower;
    table[static_cast<std::uint8_t>(InstructionType::OP_NOT)] = &ExecutionEngine::executeNotOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_NEGATE)] = &ExecutionEngine::executeNegation;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND)] = &ExecutionEngine::executeBitwiseAnd;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR)] = &ExecutionEngine::executeBitwiseOr;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR)] = &ExecutionEngine::executeBitwiseXor;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT)] = &ExecutionEngine::executeBitwiseNot;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT)] = &
            ExecutionEngine::executeBitwiseLeftShift;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT)] = &
            ExecutionEngine::executeBitwiseRightShift;

    table[static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS)] = &ExecutionEngine::executeEquality;
    table[static_cast<std::uint8_t>(InstructionType::OP_GREATER)] = &ExecutionEngine::executeGreaterOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_GREATER_EQUALS)] = &
            ExecutionEngine::executeGreaterThanOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LESS)] = &ExecutionEngine::executeLessOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LESS_EQUALS)] = &ExecutionEngine::executeLessThanOperation;

    table[static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL)] = &ExecutionEngine::executeDefineGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_DEFINE_CONST)] = &
            ExecutionEngine::executeDefineConstantVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL)] = &ExecutionEngine::executeGetGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL)] = &ExecutionEngine::executeSetGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_GET_LOCAL)] = &ExecutionEngine::executeGetLocalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL)] = &ExecutionEngine::executeSetLocalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_BUILD_ARRAY)] = &ExecutionEngine::executeBuildArray;
    table[static_cast<std::uint8_t>(InstructionType::OP_BUILD_SET)] = &ExecutionEngine::executeBuildSet;
    table[static_cast<std::uint8_t>(InstructionType::OP_BUILD_DICTIONARY)] = &ExecutionEngine::executeBuildDictionary;
    table[static_cast<std::uint8_t>(InstructionType::OP_INDEX_GET)] = &ExecutionEngine::executeGetIndex;
    table[static_cast<std::uint8_t>(InstructionType::OP_INDEX_SET)] = &ExecutionEngine::executeSetIndex;
    table[static_cast<std::uint8_t>(InstructionType::OP_GET_PROPERTY)] = &ExecutionEngine::executeGetProperty;

    table[static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE)] = &ExecutionEngine::executeJumpIfFalseOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_JUMP)] = &ExecutionEngine::executeJumpOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_POP)] = &ExecutionEngine::executePopOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LOOP)] = &ExecutionEngine::executeLoop;
    table[static_cast<std::uint8_t>(InstructionType::OP_DUPLICATE)] = &ExecutionEngine::executeDuplicate;
    table[static_cast<std::uint8_t>(InstructionType::OP_DUPLICATE2)] = &ExecutionEngine::executeDuplicate2;
    table[static_cast<std::uint8_t>(InstructionType::OP_CALL)] = &ExecutionEngine::executeFunctionCall;

    table[static_cast<std::uint8_t>(InstructionType::OP_IN)] = &ExecutionEngine::executeInOperator;
    table[static_cast<std::uint8_t>(InstructionType::OP_TYPEOF)] = &ExecutionEngine::executeTypeofOperator;
    table[static_cast<std::uint8_t>(InstructionType::OP_PRINT)] = &ExecutionEngine::executePrint;

    table[static_cast<std::uint8_t>(InstructionType::OP_RETURN)] = &ExecutionEngine::executeReturn;
    return table;
}();

ExecutionResult ExecutionEngine::execute() {
    while (true) {
        const auto result = (this->*dispatchTable[readByte()])();

        if (result == ExecutionResult::HALT) {
            return ExecutionResult::OK;
        }

        if (result != ExecutionResult::OK) {
            return result;
        }
    }
}

inline ExecutionResult ExecutionEngine::executeConstant() {
    push(readConstant());
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeTrueLiteral() {
    push(true);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeFalseLiteral() {
    push(false);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeNullLiteral() {
    push(NIL{});
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeInterpolatedStringLiteral() {
    const auto count = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());

    std::string interpolatedString;

    for (auto i = count - 1; i >= 0; --i) {
        interpolatedString += stringify(peek(i), false);
    }

    for (int i = 0; i < count; ++i) {
        pop();
    }

    push(interpolatedString);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeAddition() {
    return executeOperation("+",
        [&](const double &first, const double &second) {
            push(first + second);
            return ExecutionResult::OK;
        },
        [&](const double &first, const char &second) {
            push(static_cast<char>(first + second));
            return ExecutionResult::OK;
        },
        [&](const char &first, const double &second) {
            push(static_cast<char>(first + second));
            return ExecutionResult::OK;
        },
        [&](const std::string &first, const std::string &second) {
            push(first + second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const std::shared_ptr<Array> &second) {
            auto newArray = *first;
            for (const auto& elem : *second) {
                newArray.push_back(elem);
            }
            push(std::make_shared<Array>(newArray));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeSubtraction() {
    return executeOperation("-",
        [&](const double &first, const double &second) {
            push(first - second);
            return ExecutionResult::OK;
        },
        [&](const char &first, const double &second) {
            push(static_cast<char>(first - second));
            return ExecutionResult::OK;
        },
        [&](const double &first, const char &second) {
            push(static_cast<char>(second - first));
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            Set difference;
            for (const auto &elem : *first) {
                if (!second->contains(elem)) {
                    difference.insert(elem);
                }
            }
            push(std::make_shared<Set>(difference));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeMultiplication() {
    return executeOperation("*",
        [&](const double &first, const double &second) {
            push(first * second);
            return ExecutionResult::OK;
        },
        [&](const char &first, const double &second) {
            push(static_cast<char>(first * second));
            return ExecutionResult::OK;
        },
        [&](const double &first, const char &second) {
            push(static_cast<char>(first * second));
            return ExecutionResult::OK;
        },
        [&](const double &first, const std::shared_ptr<Array> &second) {
            Array newArray = *second;
            for (auto i = 1; i < first; ++i) {
                newArray.insert(newArray.end(), second->begin(), second->end());
            }
            push(std::make_shared<Array>(newArray));
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const double &second) {
            Array newArray = *first;
            for (auto i = 1; i < second; ++i) {
                newArray.insert(newArray.end(), first->begin(), first->end());
            }
            push(std::make_shared<Array>(newArray));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeDivision() {
    return executeOperation("/",
        [&](const double &first, const double &second) {
            push(first / second);
            return ExecutionResult::OK;
        },
        [&](const char &first, const double &second) {
            push(static_cast<char>(first / second));
            return ExecutionResult::OK;
        },
        [&](const double &first, const char &second) {
            push(static_cast<char>(second / first));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeModuloDivision() {
    return executeOperation("%",
        [&](const double &first, const double &second) {
            push(std::fmod(first, second));
            return ExecutionResult::OK;
        },
        [&](const char &first, const double &second) {
            push(static_cast<char>(first % static_cast<int>(second)));
            return ExecutionResult::OK;
        },
        [&](const double &first, const char &second) {
            push(static_cast<char>(second % static_cast<int>(first)));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executePower() {
    return executeOperation("**",
        [&](const double &first, const double &second) {
            push(std::pow(first, second));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeNotOperation() {
    if (peek(0).is<bool>()) {
        push(!pop().get<bool>());
    } else {
        reportRuntimeError("invalid operand for `!`: expected a boolean");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeNegation() {
    if (peek(0).is<double>()) {
        push(-pop().get<double>());
    } else {
        reportRuntimeError("invalid operand for unary `-`: expected a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseAnd() {
    return executeOperation("&",
        [&](const double &first, const double &second) {
            push(executeBitwiseBinaryOperation(first, second, std::bit_and<std::int64_t>{}));
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            const auto& smaller = first->size() < second->size() ? first : second;
            const auto& larger = first->size() < second->size() ? second : first;

            Set intersection;
            for (const auto& elem : *smaller) {
                if (larger->contains(elem)) {
                    intersection.insert(elem);
                }
            }

            push(std::make_shared<Set>(intersection));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeBitwiseOr() {
    return executeOperation("|",
        [&](const double &first, const double &second) {
            push(executeBitwiseBinaryOperation(first, second, std::bit_or<std::int64_t>{}));
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            Set setUnion = *first;
            setUnion.insert(second->begin(), second->end());
            push(std::make_shared<Set>(setUnion));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeBitwiseXor() {
    return executeOperation("^",
        [&](const double &first, const double &second) {
            push(executeBitwiseBinaryOperation(first, second, std::bit_xor<std::int64_t>{}));
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            Set symmetricDifference;

            for (const auto& elem : *first) {
                if (!second->contains(elem)) {
                    symmetricDifference.insert(elem);
                }
            }

            for (const auto& elem : *second) {
                if (!first->contains(elem)) {
                    symmetricDifference.insert(elem);
                }
            }

            push(std::make_shared<Set>(symmetricDifference));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeBitwiseNot() {
    if (peek(0).is<double>()) {
        const auto value = static_cast<std::int64_t>(pop().get<double>());
        push(static_cast<double>(~value));
    } else {
        reportRuntimeError("invalid operand for unary `~`: expected a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseLeftShift() {
    return executeOperation("<<",
        [&](const double &first, const double &second) {
            const auto shift = static_cast<std::int64_t>(second);
            const auto value = static_cast<std::int64_t>(first);

            push(static_cast<double>(value << (shift % 64)));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeBitwiseRightShift() {
    return executeOperation(">>",
        [&](const double &first, const double &second) {
            const auto shift = static_cast<std::int64_t>(second);
            const auto value = static_cast<std::int64_t>(first);

            push(static_cast<double>(value >> (shift % 64)));
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeEquality() {
    const auto rhs = pop();
    const auto lhs = pop();

    if (lhs != rhs) {
        reportRuntimeError("type mismatch in equality comparison: cannot compare different data types");
        return ExecutionResult::RUNTIME_ERROR;
    }

    push(lhs == rhs);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGreaterOperation() {
    return executeOperation(">",
        [&](const double &first, const double &second) {
            push(first > second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const std::shared_ptr<Array> &second) {
            push(*first > *second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            if (first->size() <= second->size()) {
                push(false);
                return ExecutionResult::OK;
            }

            bool superset = false;
            for (const auto &elem : *second) {
                if (!first->contains(elem)) {
                    superset = false;
                    break;
                }
            }
            push(superset);
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeGreaterThanOperation() {
    return executeOperation(">=",
        [&](const double &first, const double &second) {
            push(first >= second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const std::shared_ptr<Array> &second) {
            push(*first >= *second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            bool superset = false;
            for (const auto &elem : *second) {
                if (!first->contains(elem)) {
                    superset = false;
                    break;
                }
            }
            push(superset);
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeLessOperation() {
    return executeOperation("<",
        [&](const double &first, const double &second) {
            push(first < second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const std::shared_ptr<Array> &second) {
            push(*first < *second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            if (first->size() >= second->size()) {
                push(false);
                return ExecutionResult::OK;
            }

            bool subset = false;
            for (const auto &elem : *first) {
                if (!second->contains(elem)) {
                    subset = false;
                    break;
                }
            }
            push(subset);
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeLessThanOperation() {
    return executeOperation("<=",
        [&](const double &first, const double &second) {
            push(first <= second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Array> &first, const std::shared_ptr<Array> &second) {
            push(*first <= *second);
            return ExecutionResult::OK;
        },
        [&](const std::shared_ptr<Set> &first, const std::shared_ptr<Set> &second) {
            bool subset = false;
            for (const auto &elem : *first) {
                if (!second->contains(elem)) {
                    subset = false;
                    break;
                }
            }
            push(subset);
            return ExecutionResult::OK;
        }
    );
}

inline ExecutionResult ExecutionEngine::executeDefineGlobalVariable() {
    const auto name = readConstant().get<std::string>();
    globalVariables[name] = peek(0);
    pop();
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDefineConstantVariable() {
    const auto name = readConstant().get<std::string>();
    globalVariables[name] = peek(0);
    constants.insert(name);
    pop();
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetGlobalVariable() {
    const auto name = readConstant().get<std::string>();

    if (const auto it = globalVariables.find(name); it != globalVariables.end()) {
        push(it->second);
    } else {
        reportRuntimeError("undefined variable `" + name + "`");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetGlobalVariable() {
    const auto name = readConstant().get<std::string>();

    if (constants.contains(name)) {
        reportRuntimeError("cannot reassign constant variable `" + name + "`");
        return ExecutionResult::RUNTIME_ERROR;
    }

    if (const auto it = globalVariables.find(name); it != globalVariables.end()) {
        globalVariables[name] = peek(0);
    } else {
        reportRuntimeError("undefined variable `" + name + "`");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetLocalVariable() {
    const auto index = readByte();

    if (frames.empty()) {
        push(stack[index]);
    } else {
        push(stack[frames.back().stackOffset + index + 1]);
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetLocalVariable() {
    stack[readByte()] = peek(0);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBuildArray() {
    const auto count = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());
    Array array;
    array.resize(count);

    for (auto i = count - 1; i >= 0; --i) {
        array[i] = pop();
    }

    push(std::make_shared<Array>(std::move(array)));
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBuildSet() {
    const auto count = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());

    Set set;
    set.reserve(count);

    for (int i = 0; i < count; ++i) {
        set.insert(pop());
    }

    push(std::make_shared<Set>(std::move(set)));
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBuildDictionary() {
    const auto count = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());

    Dictionary dict;
    dict.reserve(count);

    for (int i = 0; i < count; ++i) {
        const auto val = pop();
        const auto key = pop();

        std::string keyStr;
        if (key.is<std::string>()) {
            keyStr = key.get<std::string>();
        } else if (key.is<double>()) {
            keyStr = stringify(key);
        } else {
            reportRuntimeError("unsupported key `" + key.get<std::string>() + "`");
            return ExecutionResult::RUNTIME_ERROR;
        }

        dict[keyStr] = val;
    }

    push(std::make_shared<Dictionary>(std::move(dict)));
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetIndex() {
    const auto secondValue = pop();

    if (const auto firstValue = pop(); firstValue.is<std::shared_ptr<Array>>()) {
        if (!secondValue.is<double>()) {
            reportRuntimeError("array index must be an integer");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto &arrayPtr = firstValue.get<std::shared_ptr<Array>>();
        const auto idx = static_cast<int>(secondValue.get<double>());

        if (idx < 0 || idx >= arrayPtr->size()) {
            reportRuntimeError("array index out of bounds");
            return ExecutionResult::RUNTIME_ERROR;
        }

        push((*arrayPtr)[idx]);
    } else if (firstValue.is<std::string>()) {
        if (!secondValue.is<double>()) {
            reportRuntimeError("string index must be an integer");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto &str = firstValue.get<std::string>();
        const auto idx = static_cast<int>(secondValue.get<double>());

        if (idx < 0 || idx >= str.size()) {
            reportRuntimeError("string index out of bounds");
            return ExecutionResult::RUNTIME_ERROR;
        }

        push(str[idx]);
    } else if (firstValue.is<std::shared_ptr<Dictionary>>()) {
        std::string key;

        if (secondValue.is<std::string>()) {
            key = secondValue.get<std::string>();
        } else if (secondValue.is<double>()) {
            key = stringify(secondValue.get<double>());
        } else {
            reportRuntimeError("dictionary key must be number or string");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto &dictionaryPtr = firstValue.get<std::shared_ptr<Dictionary>>();

        if (!dictionaryPtr->contains(key)) {
            reportRuntimeError("key `" + key + "` does not exist");
            return ExecutionResult::RUNTIME_ERROR;
        }

        push((*dictionaryPtr)[key]);
    } else {
        reportRuntimeError("an incorrect target");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetIndex() {
    const auto value = pop();
    const auto secondValue = pop();


    if (const auto firstValue = pop(); firstValue.is<std::shared_ptr<Array>>()) {
        if (!secondValue.is<double>()) {
            reportRuntimeError("array index must be a number");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto& arrayPtr = firstValue.get<std::shared_ptr<Array>>();
        const auto idx = static_cast<int>(secondValue.get<double>());

        if (idx < 0 || idx >= arrayPtr->size()) {
            reportRuntimeError("array index out of bounds");
            return ExecutionResult::RUNTIME_ERROR;
        }

        (*arrayPtr)[idx] = value;
        push(value);
    } else if (firstValue.is<std::shared_ptr<Dictionary>>()) {
        std::string key;

        if (secondValue.is<std::string>()) {
            key = secondValue.get<std::string>();
        } else if (secondValue.is<double>()) {
            key = stringify(secondValue.get<double>());
        } else {
            reportRuntimeError("dictionary key must be number or string");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto &dictionaryPtr = firstValue.get<std::shared_ptr<Dictionary>>();

        if (!dictionaryPtr->contains(key)) {
            reportRuntimeError("key `" + key + "` does not exist");
            return ExecutionResult::RUNTIME_ERROR;
        }

        (*dictionaryPtr)[key] = value;
        push(value);
    } else {
        reportRuntimeError("an incorrect target");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetProperty() {
    const auto name = readConstant().get<std::string>();

    if (const auto target = pop(); target.is<std::shared_ptr<Array>>()) {
        if (auto method = getArrayMethod(name)) {
            push(std::make_shared<BoundNativeMethod>(target, method));
            return ExecutionResult::OK;
        }
    }

    reportRuntimeError("property `" + name + "` does not exist");
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeJumpIfFalseOperation() {
    const auto offset = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());

    const auto condition = peek(0);
    bool isFalse = false;

    if (condition.is<bool>()) {
        isFalse = !condition.get<bool>();
    } else if (condition.is<NIL>()) {
        isFalse = true;
    }

    if (isFalse) {
        address += offset;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeJumpOperation() {
    address += (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executePopOperation() {
    pop();
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeLoop() {
    address -= (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDuplicate() {
    push(peek(0));
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDuplicate2() {
    const auto target = peek(1);
    const auto index = peek(0);
    push(target);
    push(index);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeFunctionCall() {
    const auto argCount = readByte();
    const auto callee = peek(argCount);

    if (callee.is<std::shared_ptr<Function>>()) {
        const auto &func = callee.get<std::shared_ptr<Function>>();

        if (argCount != func->arity) {
            reportRuntimeError(
                "expected " + std::to_string(func->arity) + " arguments but got " + std::to_string(argCount));
            return ExecutionResult::RUNTIME_ERROR;
        }

        Frame frame;
        frame.function = func;
        frame.address = address;
        frame.stackOffset = static_cast<int>(stack.size()) - argCount - 1;

        frames.push_back(frame);
        address = buffer->code.data() + func->startingAddress;

        return ExecutionResult::OK;
    }

    if (callee.is<std::shared_ptr<BoundNativeMethod>>()) {
        const auto &bound = callee.get<std::shared_ptr<BoundNativeMethod>>();
        const auto &nativeFunc = bound->method;

        if (argCount != nativeFunc->arity) {
            reportRuntimeError(
                "expected " + std::to_string(nativeFunc->arity) + " arguments but got " + std::to_string(argCount));
            return ExecutionResult::RUNTIME_ERROR;
        }

        std::vector<Value> args;
        for (auto i = argCount - 1; i >= 0; --i) {
            args.push_back(peek(i));
        }

        Value finalValue;
        try {
            finalValue = nativeFunc->callable(bound->receiver, args);
        } catch (const std::runtime_error &error) {
            reportRuntimeError(error.what());
            return ExecutionResult::RUNTIME_ERROR;
        }

        for (auto i = 0; i < argCount; ++i) {
            pop();
        }

        pop();
        push(finalValue);

        return ExecutionResult::OK;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeInOperator() {
    const auto rhs = pop();
    const auto lhs = pop();

    if (rhs.is<std::shared_ptr<Array>>()) {
        const auto &arrayPtr = rhs.get<std::shared_ptr<Array>>();
        const auto it = std::ranges::find(*arrayPtr, lhs);
        push(it != arrayPtr->end());
    } else if (rhs.is<std::shared_ptr<Set>>()) {
        const auto &setPtr = rhs.get<std::shared_ptr<Set>>();
        push(setPtr->contains(lhs));
    } else if (rhs.is<std::shared_ptr<Dictionary>>()) {
        const auto &dictionaryPtr = rhs.get<std::shared_ptr<Dictionary>>();

        std::string key;
        if (lhs.is<std::string>()) {
            key = lhs.get<std::string>();
        } else if (lhs.is<double>()) {
            key = stringify(lhs.get<double>());
        } else {
            push(false);
            return ExecutionResult::OK;
        }

        push(dictionaryPtr->contains(key));
    } else if (rhs.is<std::string>()) {
        if (!lhs.is<std::string>()) {
            reportRuntimeError("element is of type `" + stringify(lhs) + "`, but not string");
            return ExecutionResult::RUNTIME_ERROR;
        }

        const auto &stringPtr = rhs.get<std::string>();
        push(stringPtr.find(lhs.get<std::string>()) != std::string::npos);
    } else {
        reportRuntimeError("right-hand side of `in` must be container or string literal");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeTypeofOperator() {
    push(pop().type());
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executePrint() {
    std::cout << stringify(pop()) << "\n";
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeReturn() {
    if (frames.empty()) {
        if (!stack.empty()) {
            std::cout << stringify(pop()) << "\n";
        }

        return ExecutionResult::HALT;
    }

    const auto finalValue = pop();

    const auto frame = frames.back();
    frames.pop_back();
    address = frame.address;

    while (stack.size() > frame.stackOffset) {
        pop();
    }

    push(finalValue);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeUnknown() {
    return ExecutionResult::OK;
}

std::uint8_t ExecutionEngine::readByte() {
    return *address++;
}

Value ExecutionEngine::readConstant() {
    return buffer->values[readByte()];
}

template<typename... T>
ExecutionResult ExecutionEngine::executeOperation(const std::string &symbol, T... items) {
    const auto rhs = pop();
    const auto lhs = pop();

    return std::visit<ExecutionResult>(overloaded {
        items...,
        [&](const auto &first, const auto &second) {
            reportRuntimeError("unsupported operand types for `" + symbol + "`: [" + lhs.type()
                + "] and [" + rhs.type() + "]");
            return ExecutionResult::RUNTIME_ERROR;
        }
    }, lhs.as, rhs.as);
}

template<typename T>
double ExecutionEngine::executeBitwiseBinaryOperation(const double &firstNumber, const double &secondNumber,
                                                      T operation) {
    auto lhs = static_cast<std::int64_t>(firstNumber);
    auto rhs = static_cast<std::int64_t>(secondNumber);
    const std::int64_t result = operation(lhs, rhs);
    return static_cast<double>(result);
}

void ExecutionEngine::resetStack() {
    stack.clear();
}

void ExecutionEngine::push(const Value &value) {
    stack.push_back(value);
}

Value ExecutionEngine::pop() {
    auto value = stack.back();
    stack.pop_back();
    return value;
}

Value ExecutionEngine::peek(const int &distance) const {
    return stack[stack.size() - distance - 1];
}

void ExecutionEngine::registerStandardLibrary() {
    stdlib::ArrayBuiltins::registerMethods(this->arrayMethods);
}

std::shared_ptr<NativeFunction> ExecutionEngine::getArrayMethod(const std::string &name) {
    if (arrayMethods.contains(name)) {
        return arrayMethods[name];
    }

    return nullptr;
}

std::string ExecutionEngine::stringify(const Value& value, bool isNested) {
    return value.str();
}

void ExecutionEngine::reportRuntimeError(const std::string &message) {
    const auto line = buffer->at(address - buffer->code.data() - 1);

    diagnosticEngine.report(
        Diagnostic::DiagnosticKind::Fatal,
        SourceLocation{line, 1, 0, 0},
        message
    );
    diagnosticEngine.raise();

    resetStack();
}
