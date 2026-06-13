#include "stdlib/ArrayLib/ArrayBuiltins.h"

namespace stdlib::ArrayBuiltins {
    static Value retrieveArraySize(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("`length` called on non-array receiver");
        }

        return static_cast<double>(receiver.get<std::shared_ptr<Array>>()->size());
    }

    static Value getElementByIndex(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("`at` called on non-array receiver");
        }

        if (!args[0].is<double>()) {
            throw std::runtime_error("array index must be an integer");
        }

        const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
        const auto idx = static_cast<std::size_t>(args[0].get<double>());
        return (*receivedObject)[idx];
    }

    static Value clearArray(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("`clear` called on non-array receiver");
        }

        const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
        receivedObject->clear();
        return receivedObject;
    }

    static Value appendElementToArrayEnd(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("`append` called on non-array receiver");
        }

        const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
        receivedObject->push_back(args[0]);
        return NIL{};
    }

    static Value removeElementFromArrayEnd(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("`pop` called on non-array receiver");
        }

        const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
        const auto finalElement = receivedObject->back();
        receivedObject->pop_back();
        return finalElement;
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, retrieveArraySize);
        registry["at"] = std::make_shared<NativeFunction>("at", 1, getElementByIndex);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearArray);
        registry["append"] = std::make_shared<NativeFunction>("append", 1, appendElementToArrayEnd);
        registry["pop"] = std::make_shared<NativeFunction>("pop", 0, removeElementFromArrayEnd);
    }
}
