#include "backend/ExecutionEngine/ExecutionEngine.h"

ExecutionEngine::ExecutionEngine() : buffer(nullptr), address(nullptr) {
    stack.reserve(256);
}

ExecutionResult ExecutionEngine::run(const std::string& source) {

}

ExecutionResult ExecutionEngine::execute() {

}

template<typename T, typename U>
void ExecutionEngine::executeBinaryOperation(U operation) {

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
    return false;
}
