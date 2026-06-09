#include "backend/ExecutionEngine/ExecutionEngine.h"

ExecutionEngine::ExecutionEngine(std::string& source, DiagnosticEngine& diagnosticEngine)
    : diagnosticEngine(diagnosticEngine), source(source), buffer(nullptr), address(nullptr) {
    stack.reserve(256);
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
    table[static_cast<std::uint8_t>(InstructionType::OP_NIL)] = &ExecutionEngine::executeNullLiteral;

    table[static_cast<std::uint8_t>(InstructionType::OP_ADD)] = &ExecutionEngine::executeAddition;
    table[static_cast<std::uint8_t>(InstructionType::OP_MINUS)] = &ExecutionEngine::executeSubtraction;
    table[static_cast<std::uint8_t>(InstructionType::OP_STAR)] = &ExecutionEngine::executeMultiplication;
    table[static_cast<std::uint8_t>(InstructionType::OP_SLASH)] = &ExecutionEngine::executeDivision;
    table[static_cast<std::uint8_t>(InstructionType::OP_MODULO)] = &ExecutionEngine::executeModuloDivision;
    table[static_cast<std::uint8_t>(InstructionType::OP_POWER)] = &ExecutionEngine::executePower;
    table[static_cast<std::uint8_t>(InstructionType::OP_NOT)] = &ExecutionEngine::executeNotOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_NEGATE)] = &ExecutionEngine::executeNegation;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND)] = &ExecutionEngine::executeBitwiseAnd;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR)] = &ExecutionEngine::executeBitwiseOr;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR)] = &ExecutionEngine::executeBitwiseXor;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT)] = &ExecutionEngine::executeBitwiseNot;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT)] = &ExecutionEngine::executeBitwiseLeftShift;
    table[static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT)] = &ExecutionEngine::executeBitwiseRightShift;

    table[static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS)] = &ExecutionEngine::executeEquality;
    table[static_cast<std::uint8_t>(InstructionType::OP_GREATER)] = &ExecutionEngine::executeGreaterOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_GREATER_EQUALS)] = &ExecutionEngine::executeGreaterThanOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LESS)] = &ExecutionEngine::executeLessOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LESS_EQUALS)] = &ExecutionEngine::executeLessThanOperation;

    table[static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL)] = &ExecutionEngine::executeDefineGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_DEFINE_CONST)] = &ExecutionEngine::executeDefineConstantVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL)] = &ExecutionEngine::executeGetGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL)] = &ExecutionEngine::executeSetGlobalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_GET_LOCAL)] = &ExecutionEngine::executeGetLocalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL)] = &ExecutionEngine::executeSetLocalVariable;
    table[static_cast<std::uint8_t>(InstructionType::OP_BUILD_ARRAY)] = &ExecutionEngine::executeBuildArray;
    table[static_cast<std::uint8_t>(InstructionType::OP_INDEX_GET)] = &ExecutionEngine::executeGetIndex;
    table[static_cast<std::uint8_t>(InstructionType::OP_INDEX_SET)] = &ExecutionEngine::executeSetIndex;

    table[static_cast<std::uint8_t>(InstructionType::OP_JUMP_IF_FALSE)] = &ExecutionEngine::executeJumpIfFalseOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_JUMP)] = &ExecutionEngine::executeJumpOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_POP)] = &ExecutionEngine::executePopOperation;
    table[static_cast<std::uint8_t>(InstructionType::OP_LOOP)] = &ExecutionEngine::executeLoop;
    table[static_cast<std::uint8_t>(InstructionType::OP_DUPLICATE)] = &ExecutionEngine::executeDuplicate;

    table[static_cast<std::uint8_t>(InstructionType::OP_PRINT)] = &ExecutionEngine::executePrint;
    table[static_cast<std::uint8_t>(InstructionType::OP_RETURN)] = &ExecutionEngine::executeReturn;
    return table;
}();

ExecutionResult ExecutionEngine::execute() {
    while (true) {
        const auto instruction = readByte();
        if (const auto result = (this->*dispatchTable[instruction])(); result != ExecutionResult::OK) {
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
    push(NULL);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeAddition() {
    if (peek(0).type() == typeid(std::string) && peek(1).type() == typeid(std::string)) {
        const auto rhs = std::any_cast<std::string>(pop());
        const auto lhs = std::any_cast<std::string>(pop());
        push(lhs + rhs);
    } else if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>(std::plus<double>{});
    } else {
        auto getType = [](const std::any& value) {
            if (value.type() == typeid(double)) {
                return "Number";
            }

            if (value.type() == typeid(std::string)) {
                return "String";
            }

            if (value.type() == typeid(bool)) {
                return "Boolean";
            }

            if (value.type() == typeid(NULL) || !value.has_value()) {
                return "Null";
            }

            if (value.type() == typeid(char)) {
                return "Character";
            }

            return "Unknown";
        };


        const std::string rhsType = getType(pop());
        const std::string lhsType = getType(pop());
        const auto errorMessage = "unsupported operand types [" + lhsType + "] and [" + rhsType
                                        + "] for '+': expected either two numbers or two strings.";
        reportRuntimeError(errorMessage);
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSubtraction() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>(std::minus<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `-`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeMultiplication() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>(std::multiplies<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `*`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDivision() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>(std::divides<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `/`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeModuloDivision() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
            return std::fmod(lhs, rhs);
        });
    } else {
        reportRuntimeError("unsupported operand types for `%`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executePower() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
            return std::pow(lhs, rhs);
        });
    } else {
        reportRuntimeError("unsupported operand types for `**`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeNotOperation() {
    if (peek(0).type() == typeid(bool)) {
        push(!std::any_cast<bool>(pop()));
    } else {
        reportRuntimeError("invalid operand for `!`: expected a boolean");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeNegation() {
    if (peek(0).type() == typeid(double)) {
        push(-std::any_cast<double>(pop()));
    } else {
        reportRuntimeError("invalid operand for unary `-`: expected a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseAnd() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
            return executeBitwiseBinaryOperation(lhs, rhs, std::bit_and<std::int64_t>{});
        });
    } else {
        reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseOr() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
            return executeBitwiseBinaryOperation(lhs, rhs, std::bit_or<std::int64_t>{});
        });
    } else {
        reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseXor() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
            return executeBitwiseBinaryOperation(lhs, rhs, std::bit_xor<std::int64_t>{});
        });
    } else {
        reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseNot() {
    if (peek(0).type() == typeid(double)) {
        const auto value = static_cast<std::int64_t>(std::any_cast<double>(pop()));
        push(static_cast<double>(~value));
    } else {
        reportRuntimeError("invalid operand for unary `~`: expected a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseLeftShift() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
            return executeBitwiseBinaryOperation(lhs, rhs, [](const std::int64_t& value, const std::int64_t& shift) {
                return value << (shift % 64);
            });
        });
    } else {
        reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBitwiseRightShift() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
            return executeBitwiseBinaryOperation(lhs, rhs, [](const std::int64_t& value, const std::int64_t& shift) {
                return value >> (shift % 64);
            });
        });
    } else {
        reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeEquality() {
    const auto rhs = pop();
    const auto lhs = pop();

    if (lhs.type() != rhs.type()) {
        reportRuntimeError("type mismatch in equality comparison: cannot compare different data types");
        return ExecutionResult::RUNTIME_ERROR;
    }

    bool flag = false;
    if (lhs.type() == typeid(double)) {
        flag = std::any_cast<double>(lhs) == std::any_cast<double>(rhs);
    } else if (lhs.type() == typeid(bool)) {
        flag = std::any_cast<bool>(lhs) == std::any_cast<bool>(rhs);
    } else if (lhs.type() == typeid(std::string)) {
        flag = std::any_cast<std::string>(lhs) == std::any_cast<std::string>(rhs);
    }

    push(flag);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGreaterOperation() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<bool>(std::greater<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `>`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGreaterThanOperation() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<bool>(std::greater_equal<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `>=`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeLessOperation() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<bool>(std::less<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `<`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeLessThanOperation() {
    if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
        executeBinaryOperation<bool>(std::less_equal<double>{});
    } else {
        reportRuntimeError("unsupported operand types for `<=`: expected two numbers");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDefineGlobalVariable() {
    const auto name = std::any_cast<std::string>(readConstant());
    globalVariables[name] = peek(0);
    pop();
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeDefineConstantVariable() {
    const auto name = std::any_cast<std::string>(readConstant());
    globalVariables[name] = peek(0);
    constants.insert(name);
    pop();
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetGlobalVariable() {
    const auto name = std::any_cast<std::string>(readConstant());

    if (const auto it = globalVariables.find(name); it != globalVariables.end()) {
        push(it->second);
    } else {
        reportRuntimeError("undefined variable `" + name + "`");
        return ExecutionResult::RUNTIME_ERROR;
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetGlobalVariable() {
    const auto name = std::any_cast<std::string>(readConstant());

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
    push(stack[readByte()]);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetLocalVariable() {
    stack[readByte()] = peek(0);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeBuildArray() {
    const auto count = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());
    std::vector<std::any> array;
    array.resize(count);

    for (auto i = count - 1; i >= 0; --i) {
        array[i] = pop();
    }

    push(std::make_shared<std::vector<std::any>>(std::move(array)));
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeGetIndex() {
    const auto index = pop();
    const auto array = pop();

    if (array.type() != typeid(std::shared_ptr<std::vector<std::any>>)) {
        reportRuntimeError("target is not an array");
        return ExecutionResult::RUNTIME_ERROR;
    }

    if (index.type() != typeid(double)) {
        reportRuntimeError("array index must be a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    const auto arrayPtr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(array);
    const auto idx = static_cast<int>(std::any_cast<double>(index));

    if (idx < 0 || idx >= arrayPtr->size()) {
        reportRuntimeError("array index out of bounds");
        return ExecutionResult::RUNTIME_ERROR;
    }

    push((*arrayPtr)[idx]);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeSetIndex() {
    const auto value = pop();
    const auto index = pop();
    const auto array = pop();

    if (array.type() != typeid(std::shared_ptr<std::vector<std::any>>)) {
        reportRuntimeError("target is not an array");
        return ExecutionResult::RUNTIME_ERROR;
    }

    if (index.type() != typeid(double)) {
        reportRuntimeError("array index must be a number");
        return ExecutionResult::RUNTIME_ERROR;
    }

    const auto arrayPtr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(array);
    const auto idx = static_cast<int>(std::any_cast<double>(index));

    if (idx < 0 || idx >= arrayPtr->size()) {
        reportRuntimeError("array index out of bounds");
        return ExecutionResult::RUNTIME_ERROR;
    }

    (*arrayPtr)[idx] = value;
    push(value);
    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeJumpIfFalseOperation() {
    const auto offset = (static_cast<std::uint16_t>(readByte()) << 8) | static_cast<std::uint16_t>(readByte());

    const auto condition = peek(0);
    bool isFalse = false;

    if (condition.type() == typeid(bool)) {
        isFalse = !std::any_cast<bool>(condition);
    } else if (condition.type() == typeid(NULL) || !condition.has_value()) {
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

inline ExecutionResult ExecutionEngine::executePrint() {
    if (const auto result = pop(); result.type() == typeid(double)) {
        std::cout << std::any_cast<double>(result) << "\n";
    } else if (result.type() == typeid(std::string)) {
        std::cout << std::any_cast<std::string>(result) << "\n";
    } else if (result.type() == typeid(bool)) {
        const auto booleanResult = std::any_cast<bool>(result);
        std::cout << (booleanResult ? "True" : "False") << "\n";
    } else if (result.type() == typeid(NULL) || !result.has_value()) {
        std::cout << "null\n";
    } else if (result.type() == typeid(std::shared_ptr<std::vector<std::any>>)) {
        const auto arrayPtr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(result);

        auto printArrayElement = [](const std::any& elem) {
            if (elem.type() == typeid(double)) {
                std::cout << std::any_cast<double>(elem) << ", ";
            } else if (elem.type() == typeid(std::string)) {
                std::cout << std::any_cast<std::string>(elem) << ", ";
            } else if (elem.type() == typeid(bool)) {
                const auto booleanResult = std::any_cast<bool>(elem);
                std::cout << (booleanResult ? "True" : "False") << ", ";
            } else if (elem.type() == typeid(NULL) || !elem.has_value()) {
                std::cout << "null, ";
            }
        };

        for (const auto& element : *arrayPtr) {
            printArrayElement(element);
        }
        std::cout << "\n";
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeReturn() {
    if (!stack.empty()) {
        if (const auto result = pop(); result.type() == typeid(double)) {
            std::cout << std::any_cast<double>(result) << "\n";
        } else if (result.type() == typeid(std::string)) {
            std::cout << std::any_cast<std::string>(result) << "\n";
        } else if (result.type() == typeid(bool)) {
            const auto booleanResult = std::any_cast<bool>(result);
            std::cout << (booleanResult ? "True" : "False") << "\n";
        } else if (result.type() == typeid(NULL) || !result.has_value()) {
            std::cout << "null\n";
        } else if (result.type() == typeid(std::shared_ptr<std::vector<std::any>>)) {
            const auto arrayPtr = std::any_cast<std::shared_ptr<std::vector<std::any>>>(result);

            auto printArrayElement = [](const std::any& elem) {
                if (elem.type() == typeid(double)) {
                    std::cout << std::any_cast<double>(elem) << ", ";
                } else if (elem.type() == typeid(std::string)) {
                    std::cout << std::any_cast<std::string>(elem) << ", ";
                } else if (elem.type() == typeid(bool)) {
                    const auto booleanResult = std::any_cast<bool>(elem);
                    std::cout << (booleanResult ? "True" : "False") << ", ";
                } else if (elem.type() == typeid(NULL) || !elem.has_value()) {
                    std::cout << "null, ";
                }
            };

            for (const auto& element : *arrayPtr) {
                printArrayElement(element);
            }
            std::cout << "\n";
        }
    }

    return ExecutionResult::OK;
}

inline ExecutionResult ExecutionEngine::executeUnknown() {
    return ExecutionResult::OK;
}

std::uint8_t ExecutionEngine::readByte() {
    return *address++;
}

std::any ExecutionEngine::readConstant() {
    return buffer->values[readByte()];
}

template<typename T, typename U>
void ExecutionEngine::executeBinaryOperation(U operation) {
    if (peek(0).type() != typeid(double) || peek(1).type() != typeid(double)) {
        return;
    }

    const auto rhs = std::any_cast<double>(pop());
    const auto lhs = std::any_cast<double>(pop());
    push(static_cast<T>(operation(lhs, rhs)));
}

template<typename T>
double ExecutionEngine::executeBitwiseBinaryOperation(const double& firstNumber, const double& secondNumber, T operation) {
    auto lhs = static_cast<std::int64_t>(firstNumber);
    auto rhs = static_cast<std::int64_t>(secondNumber);
    const std::int64_t result = operation(lhs, rhs);
    return static_cast<double>(result);
}

void ExecutionEngine::resetStack() {
    stack.clear();
}

void ExecutionEngine::push(const std::any& value) {
    stack.push_back(value);
}

std::any ExecutionEngine::pop() {
    const auto value = stack.back();
    stack.pop_back();
    return value;
}

std::any ExecutionEngine::peek(const int& distance) const {
    return stack[stack.size() - distance - 1];
}

void ExecutionEngine::reportRuntimeError(const std::string& message) {
    const auto line = buffer->at(address - buffer->code.data() - 1);
    diagnosticEngine.report(
        Diagnostic::DiagnosticKind::Fatal,
        SourceLocation{ line, 1, 0, 0 },
        message
    );
    diagnosticEngine.raise();

    resetStack();
}
