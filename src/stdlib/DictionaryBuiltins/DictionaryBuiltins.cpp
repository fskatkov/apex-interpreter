#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"

namespace stdlib::DictionaryBuiltins {
    static Value getDictionaryLength(Value receiver, const std::vector<Value> &args) {
        return static_cast<double>(receiver.get<std::shared_ptr<Dictionary>>()->size());
    }

    static Value createDictionaryCopy(Value receiver, const std::vector<Value> &args) {
        return receiver.get<std::shared_ptr<Dictionary>>();
    }

    static Value clearDictionary(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        receivedObject->clear();
        return receivedObject;
    }

    static Value checkDictionaryEmptiness(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        return receivedObject->empty();
    }

    static Value getDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        auto defaultValue = args.size() > 1 ? args[1] : NIL{};

        if (auto it = receivedObject->find(args[0]); it != receivedObject->end()) {
            return it->second;
        }

        return defaultValue;
    }

    static Value setDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        const auto &key = args[0];
        auto defaultValue = args.size() > 1 ? args[1] : NIL{};

        if (auto it = receivedObject->find(key); it != receivedObject->end()) {
            return it->second;
        }

        (*receivedObject)[key] = defaultValue;
        return defaultValue;
    }

    static Value retrieveDictionaryKeys(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        Array keys;
        keys.reserve(receivedObject->size());
        std::ranges::copy(std::views::keys(*receivedObject), std::back_inserter(keys));
        return std::make_shared<Array>(keys);
    }

    static Value retrieveDictionaryValues(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        Array values;
        values.reserve(receivedObject->size());
        std::ranges::copy(std::views::values(*receivedObject), std::back_inserter(values));
        return std::make_shared<Array>(values);
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, getDictionaryLength);
        registry["copy"] = std::make_shared<NativeFunction>("copy", 0, createDictionaryCopy);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearDictionary);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkDictionaryEmptiness);

        registry["get"] = std::make_shared<NativeFunction>("get", -1, getDictionaryElementByKey);
        registry["setDefault"] = std::make_shared<NativeFunction>("setDefault", -1, setDictionaryElementByKey);
        registry["keys"] = std::make_shared<NativeFunction>("keys", 0, retrieveDictionaryKeys);
        registry["values"] = std::make_shared<NativeFunction>("values", 0, retrieveDictionaryValues);
    }
}