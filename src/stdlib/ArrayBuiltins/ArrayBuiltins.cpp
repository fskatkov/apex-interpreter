#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"

namespace stdlib::ArrayBuiltins {
    namespace {
        Value retrieveArraySize(Value receiver, const std::vector<Value> &args) {
            return static_cast<double>(receiver.get<std::shared_ptr<Array>>()->size());
        }

        Value clearArray(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            receivedObject->clear();
            return receivedObject;
        }

        Value createArrayClone(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Array>>();
        }

        Value reverseArray(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            std::ranges::reverse(*receivedObject);
            return NIL{};
        }

        Value checkArrayEmptiness(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            return receivedObject->empty();
        }

        Value getFrontArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            if (receivedObject->empty()) {
                throw std::runtime_error("no first element, an array is empty");
            }
            return receivedObject->front();
        }

        Value getBackArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            if (receivedObject->empty()) {
                throw std::runtime_error("no last element, an array is empty");
            }
            return receivedObject->back();
        }

        Value getElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<double>()) {
                throw std::runtime_error("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());
            return (*receivedObject)[idx];
        }

        Value updateElementByIndex(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<double>()) {
                throw std::runtime_error("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args[0].get<double>());

            if (idx <= 0 || idx >= receivedObject->size()) {
                throw std::runtime_error("array index out of bounds");
            }

            (*receivedObject)[idx] = args[1];
            return receivedObject;
        }

        Value appendElementToArrayEnd(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            receivedObject->push_back(args[0]);
            return NIL{};
        }

        Value appendElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<double>()) {
                throw std::runtime_error("array index must be a number");
            }

            const auto idx = static_cast<int>(args[0].get<double>());

            if (idx <= 0 || idx >= receivedObject->size()) {
                throw std::runtime_error("array index out of bounds");
            }

            receivedObject->insert(receivedObject->begin() + idx, args[1]);
            return NIL{};
        }

        Value removeElementFromArrayEnd(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            const auto finalElement = receivedObject->back();
            receivedObject->pop_back();
            return finalElement;
        }

        Value removeElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<double>()) {
                throw std::runtime_error("array index must be a number");
            }

            const auto idx = static_cast<int>(args[0].get<double>());

            if (idx <= 0 || idx >= receivedObject->size()) {
                throw std::runtime_error("array index out of bounds");
            }

            receivedObject->erase(receivedObject->begin() + idx);
            return NIL{};
        }

        Value sliceArray(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<double>() || !args[1].is<double>()) {
                throw std::runtime_error("array index must be a number");
            }

            const auto startingIndex = static_cast<int>(args[0].get<double>());
            const auto finalIndex = static_cast<int>(args[1].get<double>());

            Array slicedArray(receivedObject->begin() + startingIndex, receivedObject->begin() + finalIndex + 1);
            return std::make_shared<Array>(slicedArray);
        }

        Value concatenateTwoArrays(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            if (!args[0].is<std::shared_ptr<Array>>()) {
                throw std::runtime_error("cannot concatenate array and " + args[0].type());
            }

            const auto anotherArray = args[0].get<std::shared_ptr<Array>>();
            receivedObject->insert(receivedObject->end(), anotherArray->begin(), anotherArray->end());
            return NIL{};
        }

        Value retrieveFirstIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            const auto it = std::ranges::find(*receivedObject, args[0]);
            if (it == receivedObject->end()) {
                throw std::runtime_error("not found element " + it->str());
            }

            const auto index = std::distance(receivedObject->begin(), it);
            return static_cast<double>(index);
        }

        Value retrieveLastIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();

            const auto it = std::find(receivedObject->rbegin(), receivedObject->rend(), args[0]);
            if (it == receivedObject->rend()) {
                throw std::runtime_error("not found element " + it->str());
            }

            const auto index = receivedObject->size() - 1 - std::distance(receivedObject->rbegin(), it);
            return static_cast<double>(index);
        }

        Value containsValue(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
            const auto it = std::ranges::find(*receivedObject, args[0]);
            return it != receivedObject->end();
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction>> table;
        table["len"] = std::make_shared<NativeFunction>("len", 0, retrieveArraySize);
        table["clear"] = std::make_shared<NativeFunction>("clear", 0, clearArray);
        table["copy"] = std::make_shared<NativeFunction>("copy", 0, createArrayClone);
        table["reverse"] = std::make_shared<NativeFunction>("reverse", 0, reverseArray);
        table["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkArrayEmptiness);
        table["first"] = std::make_shared<NativeFunction>("first", 0, getFrontArrayElement);
        table["last"] = std::make_shared<NativeFunction>("last", 0, getBackArrayElement);
        table["at"] = std::make_shared<NativeFunction>("at", 1, getElementByIndex);
        table["set"] = std::make_shared<NativeFunction>("set", 2, updateElementByIndex);
        table["append"] = std::make_shared<NativeFunction>("append", 1, appendElementToArrayEnd);
        table["insertAt"] = std::make_shared<NativeFunction>("insertAt", 2, appendElementByIndex);
        table["pop"] = std::make_shared<NativeFunction>("pop", 0, removeElementFromArrayEnd);
        table["removeAt"] = std::make_shared<NativeFunction>("removeAt", 1, removeElementByIndex);
        table["slice"] = std::make_shared<NativeFunction>("slice", 2, sliceArray);
        table["concat"] = std::make_shared<NativeFunction>("concat", 1, concatenateTwoArrays);
        table["indexOf"] = std::make_shared<NativeFunction>("indexOf", 1, retrieveFirstIndexOfArrayElement);
        table["lastIndexOf"] = std::make_shared<NativeFunction>("lastIndexOf", 1, retrieveLastIndexOfArrayElement);
        table["contains"] = std::make_shared<NativeFunction>("contains", 1, containsValue);
        return table;
    }
}
