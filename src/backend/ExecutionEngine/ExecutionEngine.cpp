#include "backend/ExecutionEngine/ExecutionEngine.h"

ExecutionEngine::ExecutionEngine() : buffer(nullptr), address(nullptr) {
    stack.reserve(256);
}

ExecutionResult ExecutionEngine::run(const std::string& source) {
    return execute();
}

ExecutionResult ExecutionEngine::execute() {
    while (true) {
        switch (const auto instruction = readByte(); instruction) {
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
    if (peek(0).type() != typeid(double) && peek(1).type() != typeid(double)) {
        return;
    }

    const auto rhs = std::any_cast<double>(pop());
    const auto lhs = std::any_cast<double>(pop());
    push(static_cast<T>(operation(lhs, rhs)));
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
