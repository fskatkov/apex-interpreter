#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"

namespace stdlib::ArrayBuiltins {
    namespace {
        Value retrieveArraySize(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return static_cast<double>(receivedObject->size());
        }

        Value clearArray(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            receivedObject->clear();
            return receivedObject;
        }

        Value createArrayClone(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return std::make_shared<Array>(*receivedObject);
        }

        Value reverseArray(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            std::ranges::reverse(*receivedObject);
            return NIL{};
        }

        Value checkArrayEmptiness(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return receivedObject->empty();
        }

        Value getFrontArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the front element");
            }
            return receivedObject->front();
        }

        Value getBackArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the back element");
            }
            return receivedObject->back();
        }

        Value getElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            return (*receivedObject)[idx];
        }

        Value updateElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            (*receivedObject)[idx] = args[1];
            return receivedObject;
        }

        Value appendElementToArrayEnd(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            receivedObject->push_back(args[0]);
            return NIL{};
        }

        Value appendElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());
            if (idx > receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            receivedObject->insert(receivedObject->begin() + idx, args[1]);
            return NIL{};
        }

        Value removeElementFromArrayEnd(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("cannot pop from an empty array");
            }

            auto backElement = std::move(receivedObject->back());
            receivedObject->pop_back();
            return backElement;
        }

        Value removeElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            receivedObject->erase(receivedObject->begin() + idx);
            return NIL{};
        }

        Value sliceArray(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<double>() || !args[1].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            if (args[0].get<double>() < 0 || args[1].get<double>() < 0) [[unlikely]] {
                throw std::out_of_range("array indices must be non-negative");
            }

            const auto startingIndex = static_cast<std::size_t>(args[0].get<double>());
            const auto finalIndex = static_cast<std::size_t>(args[1].get<double>());

            if (startingIndex >= receivedObject->size() || startingIndex > finalIndex) [[unlikely]] {
                return std::make_shared<Array>();
            }

            const auto resultingIndex = std::min(receivedObject->size() - 1, finalIndex);

            return std::make_shared<Array>(
                receivedObject->begin() + startingIndex,
                receivedObject->begin() + resultingIndex + 1
            );
        }

        Value concatenateTwoArrays(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args[0].is<std::shared_ptr<Array> >()) [[unlikely]] {
                throw std::invalid_argument(std::format("cannot concatenate array and {}", args[0].type()));
            }

            const auto &anotherArray = args[0].get<std::shared_ptr<Array> >();
            receivedObject->reserve(receivedObject->size() + anotherArray->size());
            receivedObject->insert(receivedObject->end(), anotherArray->begin(), anotherArray->end());
            return NIL{};
        }

        Value retrieveFirstIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            const auto it = std::ranges::find(*receivedObject, args[0]);
            if (it == receivedObject->end()) [[unlikely]] {
                throw std::runtime_error(std::format("not found element {}", it->str()));
            }

            return static_cast<double>(std::ranges::distance(receivedObject->begin(), it));
        }

        Value retrieveLastIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            const auto it = std::ranges::find(std::ranges::rbegin(*receivedObject), std::ranges::rend(*receivedObject),
                                              args[0]);
            if (it == receivedObject->rend()) {
                throw std::runtime_error(std::format("not found element {}", it->str()));
            }

            const auto index = receivedObject->size() - 1 - std::ranges::distance(
                                   std::ranges::rbegin(*receivedObject), it);
            return static_cast<double>(index);
        }

        Value containsValue(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return std::ranges::find(*receivedObject, args[0]) != receivedObject->end();
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            { "len", std::make_shared<NativeFunction>("len", 0, retrieveArraySize) },
            { "clear", std::make_shared<NativeFunction>("clear", 0, clearArray) },
            { "copy", std::make_shared<NativeFunction>("copy", 0, createArrayClone) },
            { "reverse", std::make_shared<NativeFunction>("reverse", 0, reverseArray) },
            { "isEmpty", std::make_shared<NativeFunction>("isEmpty", 0, checkArrayEmptiness) },
            { "first", std::make_shared<NativeFunction>("first", 0, getFrontArrayElement) },
            { "last", std::make_shared<NativeFunction>("last", 0, getBackArrayElement) },
            { "at", std::make_shared<NativeFunction>("at", 1, getElementByIndex) },
            { "set", std::make_shared<NativeFunction>("set", 2, updateElementByIndex) },
            { "append", std::make_shared<NativeFunction>("append", 1, appendElementToArrayEnd) },
            { "insertAt", std::make_shared<NativeFunction>("insertAt", 2, appendElementByIndex) },
            { "pop", std::make_shared<NativeFunction>("pop", 0, removeElementFromArrayEnd) },
            { "removeAt", std::make_shared<NativeFunction>("removeAt", 1, removeElementByIndex) },
            { "slice", std::make_shared<NativeFunction>("slice", 2, sliceArray) },
            { "concat", std::make_shared<NativeFunction>("concat", 1, concatenateTwoArrays) },
            { "indexOf", std::make_shared<NativeFunction>("indexOf", 1, retrieveFirstIndexOfArrayElement) },
            { "lastIndexOf", std::make_shared<NativeFunction>("lastIndexOf", 1, retrieveLastIndexOfArrayElement) },
            { "contains", std::make_shared<NativeFunction>("contains", 1, containsValue) },
        };
    }
}
