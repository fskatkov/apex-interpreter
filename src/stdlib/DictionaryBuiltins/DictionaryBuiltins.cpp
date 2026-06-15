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

    static Value containsValue(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        if (!args[0].is<double>() && !args[0].is<std::string>()) {
            throw std::runtime_error("dictionary key must be number or string");
        }
        return receivedObject->contains(args[0]);
    }

    static Value getDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
        auto defaultValue = args.size() > 1 ? args[1] : NIL{};

        if (!args[0].is<double>() && !args[0].is<std::string>()) {
            throw std::runtime_error("dictionary key must be number or string");
        }

        if (auto it = receivedObject->find(args[0]); it != receivedObject->end()) {
            return it->second;
        }

        return defaultValue;
    }

    static Value setDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

        if (!args[0].is<double>() && !args[0].is<std::string>()) {
            throw std::runtime_error("dictionary key must be number or string");
        }

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

    static Value mergeDictionaries(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

        if (!args[0].is<std::shared_ptr<Dictionary>>()) {
            throw std::runtime_error("expected `Dictionary` but got `" + args[0].type() + "`");
        }

        const auto otherObject = args[0].get<std::shared_ptr<Dictionary>>();
        receivedObject->merge(*otherObject);
        return receivedObject;
    }

    static Value insertDictionary(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

        if (!args[0].is<std::shared_ptr<Dictionary>>()) {
            throw std::runtime_error("expected `Dictionary` but got `" + args[0].type() + "`");
        }

        const auto otherObject = args[0].get<std::shared_ptr<Dictionary>>();
        receivedObject->insert(otherObject->begin(), otherObject->end());
        return receivedObject;
    }

    static Value removeDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

        if (!args[0].is<double>() && !args[0].is<std::string>()) {
            throw std::runtime_error("dictionary key must be number or string");
        }

        const auto dictionaryElement = receivedObject->extract(args[0]);
        if (!dictionaryElement) {
            throw std::runtime_error("dictionary element does not exist");
        }

        return dictionaryElement.mapped();
    }

    static Value createDictionaryFromKeys(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

        if (!args[0].is<std::shared_ptr<Array>>()) {
            throw std::runtime_error("expected an array of keys, but got `" + args[0].type() + "`");
        }

        const auto keysArray = args[0].get<std::shared_ptr<Array>>();

        receivedObject->clear();
        receivedObject->reserve(keysArray->size());

        for (const auto &key : *keysArray) {
            receivedObject->emplace(key, args[1]);
        }

        return receivedObject;
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, getDictionaryLength);
        registry["copy"] = std::make_shared<NativeFunction>("copy", 0, createDictionaryCopy);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearDictionary);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkDictionaryEmptiness);
        registry["contains"] = std::make_shared<NativeFunction>("contains", 1, containsValue);

        registry["get"] = std::make_shared<NativeFunction>("get", -1, getDictionaryElementByKey);
        registry["setDefault"] = std::make_shared<NativeFunction>("setDefault", -1, setDictionaryElementByKey);
        registry["keys"] = std::make_shared<NativeFunction>("keys", 0, retrieveDictionaryKeys);
        registry["values"] = std::make_shared<NativeFunction>("values", 0, retrieveDictionaryValues);

        registry["merge"] = std::make_shared<NativeFunction>("merge", 1, mergeDictionaries);
        registry["insert"] = std::make_shared<NativeFunction>("insert", 1, insertDictionary);
        registry["pop"] = std::make_shared<NativeFunction>("pop", 1, removeDictionaryElementByKey);
        registry["fromKeys"] = std::make_shared<NativeFunction>("fromKeys", 2, createDictionaryFromKeys);
    }
}