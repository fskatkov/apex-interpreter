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

ExecutionResult ExecutionEngine::execute() {
    while (true) {
        switch (const auto instruction = readByte(); instruction) {
            case static_cast<std::uint8_t>(InstructionType::OP_CONSTANT): {
                push(readConstant());
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_TRUE): {
                push(true);
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_FALSE): {
                push(false);
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_NIL): {
                push(NULL);
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_ADD): {
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
                        } else if (value.type() == typeid(std::string)) {
                            return "String";
                        } else if (value.type() == typeid(bool)) {
                            return "Boolean";
                        } else if (value.type() == typeid(NULL) || !value.has_value()) {
                            return "Null";
                        } else if (value.type() == typeid(char)) {
                            return "Character";
                        }

                        return "Unknown";
                    };


                    const std::string rhsType = getType(pop());
                    const std::string lhsType = getType(pop());
                    auto errorMessage = "unsupported operand types [" + lhsType + "] and [" + rhsType + "] for '+': expected either two numbers or two strings.";
                    reportRuntimeError(errorMessage);
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_MINUS): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::minus<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `-`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_STAR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::multiplies<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `*`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_SLASH): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::divides<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `/`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_MODULO): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
                        return std::fmod(lhs, rhs);
                    });
                } else {
                    reportRuntimeError("unsupported operand types for `%`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_POWER): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
                        return std::pow(lhs, rhs);
                    });
                } else {
                    reportRuntimeError("unsupported operand types for `**`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_NOT): {
                if (peek(0).type() == typeid(bool)) {
                    push(!std::any_cast<bool>(pop()));
                } else {
                    reportRuntimeError("invalid operand for `!`: expected a boolean");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_NEGATE): {
                if (peek(0).type() == typeid(double)) {
                    push(-std::any_cast<double>(pop()));
                } else {
                    reportRuntimeError("invalid operand for unary `-`: expected a number");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_and<std::int64_t>{});
                    });
                } else {
                    reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_or<std::int64_t>{});
                    });
                } else {
                    reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_xor<std::int64_t>{});
                    });
                } else {
                    reportRuntimeError("invalid operand types for bitwise operation: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT): {
                if (peek(0).type() == typeid(double)) {
                    const auto value = static_cast<std::int64_t>(std::any_cast<double>(pop()));
                    push(static_cast<double>(~value));
                } else {
                    reportRuntimeError("invalid operand for unary `~`: expected a number");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT): {
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

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT): {
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

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_EQUALS_EQUALS): {
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
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_GREATER): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<bool>(std::greater<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `>`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_GREATER_EQUALS): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<bool>(std::greater_equal<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `>=`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_LESS): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<bool>(std::less<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `<`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_LESS_EQUALS): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<bool>(std::less_equal<double>{});
                } else {
                    reportRuntimeError("unsupported operand types for `<=`: expected two numbers");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_DEFINE_GLOBAL): {
                const auto name = std::any_cast<std::string>(readConstant());
                globalVariables[name] = peek(0);
                pop();
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_DEFINE_CONST): {
                const auto name = std::any_cast<std::string>(readConstant());
                globalVariables[name] = peek(0);
                constants.insert(name);
                pop();
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_GET_GLOBAL): {
                const auto name = std::any_cast<std::string>(readConstant());

                if (auto it = globalVariables.find(name); it != globalVariables.end()) {
                    push(it->second);
                } else {
                    reportRuntimeError("undefined variable `" + name + "`");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_SET_GLOBAL): {
                const auto name = std::any_cast<std::string>(readConstant());

                if (constants.contains(name)) {
                    reportRuntimeError("cannot reassign constant variable `" + name + "`");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                if (auto it = globalVariables.find(name); it != globalVariables.end()) {
                    globalVariables[name] = peek(0);
                } else {
                    reportRuntimeError("undefined variable `" + name + "`");
                    return ExecutionResult::RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_GET_LOCAL): {
                push(stack[readByte()]);
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_SET_LOCAL): {
                stack[readByte()] = peek(0);
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_PRINT): {
                if (const auto result = pop(); result.type() == typeid(double)) {
                    std::cout << std::any_cast<double>(result) << "\n";
                } else if (result.type() == typeid(std::string)) {
                    std::cout << std::any_cast<std::string>(result) << "\n";
                } else if (result.type() == typeid(bool)) {
                    const auto booleanResult = std::any_cast<bool>(result);
                    std::cout << (booleanResult ? "True" : "False") << "\n";
                } else if (result.type() == typeid(NULL) || !result.has_value()) {
                    std::cout << "null\n";
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_POP): {
                pop();
                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_RETURN): {
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
                    }
                }

                return ExecutionResult::OK;
            }
            default:
                return ExecutionResult::OK;
        }
    }
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
