#include "backend/ExecutionEngine/ExecutionEngine.h"

ExecutionEngine::ExecutionEngine() : buffer(nullptr), address(nullptr) {
    stack.reserve(256);
}

ExecutionResult ExecutionEngine::run(std::string& source) {
    DiagnosticEngine diagnosticEngine(source);
    BytecodeGenerator generator(diagnosticEngine);

    auto compiledBuffer = generator.generate(source);
    if (!compiledBuffer) {
        return ExecutionResult::INTERPRETER_COMPILE_ERROR;
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
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_MINUS): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::minus<double>{});
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_STAR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::multiplies<double>{});
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_SLASH): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>(std::divides<double>{});
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_MODULO): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
                        return std::fmod(lhs, rhs);
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_POWER): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([](const double& lhs, const double& rhs) {
                        return std::pow(lhs, rhs);
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_AND): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_and<std::uint64_t>{});
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_OR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_or<std::uint64_t>{});
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_XOR): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, std::bit_xor<std::uint64_t>{});
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_NOT): {
                if (peek(0).type() == typeid(double)) {
                    const auto value = static_cast<std::uint64_t>(std::any_cast<double>(pop()));
                    push(static_cast<double>(~value));
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_LEFT_SHIFT): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, [](const std::uint64_t& value, const std::uint64_t& shift) {
                            return value << (shift % 64);
                        });
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_BITWISE_RIGHT_SHIFT): {
                if (peek(0).type() == typeid(double) && peek(1).type() == typeid(double)) {
                    executeBinaryOperation<double>([this](const double& lhs, const double& rhs) {
                        return executeBitwiseBinaryOperation(lhs, rhs, [](const std::uint64_t& value, const std::uint64_t& shift) {
                            return value >> (shift % 64);
                        });
                    });
                } else {
                    return ExecutionResult::INTERPRETER_RUNTIME_ERROR;
                }

                break;
            }
            case static_cast<std::uint8_t>(InstructionType::OP_RETURN): {
                if (!stack.empty()) {
                    if (const auto result = pop(); result.type() == typeid(double)) {
                        std::cout << std::any_cast<double>(result) << "\n";
                    } else if (result.type() == typeid(std::string)) {
                        std::cout << std::any_cast<std::string>(result) << "\n";
                    } else if (result.type() == typeid(bool)) {
                        std::cout << (std::any_cast<bool>(result) ? "True" : "False") << "\n";
                    } else if (result.type() == typeid(NULL) || !result.has_value()) {
                        std::cout << "null\n";
                    }
                }

                return ExecutionResult::INTERPRETER_OK;
            }
            default:
                return ExecutionResult::INTERPRETER_OK;
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
    auto lhs = static_cast<std::uint64_t>(firstNumber);
    auto rhs = static_cast<std::uint64_t>(secondNumber);
    const std::uint64_t result = operation(lhs, rhs);
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

bool ExecutionEngine::isNegative(const std::any& value) const {
    return !value.has_value() || (value.type() == typeid(bool) && std::any_cast<bool>(value) == false);
}
