#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"

namespace stdlib::DictionaryBuiltins {
    namespace {
        Value getDictionaryLength(Value receiver, const std::vector<Value> &args) {
            return static_cast<double>(receiver.get<std::shared_ptr<Dictionary>>()->size());
        }

        Value createDictionaryCopy(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Dictionary>>();
        }

        Value clearDictionary(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
            receivedObject->clear();
            return receivedObject;
        }

        Value checkDictionaryEmptiness(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
            return receivedObject->empty();
        }

        Value containsValue(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
            if (!args[0].is<double>() && !args[0].is<std::string>()) {
                throw std::runtime_error("dictionary key must be number or string");
            }
            return receivedObject->contains(args[0]);
        }

        Value getDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
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

        Value setDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
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

        Value retrieveDictionaryKeys(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
            Array keys;
            keys.reserve(receivedObject->size());
            std::ranges::copy(std::views::keys(*receivedObject), std::back_inserter(keys));
            return std::make_shared<Array>(keys);
        }

        Value retrieveDictionaryValues(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();
            Array values;
            values.reserve(receivedObject->size());
            std::ranges::copy(std::views::values(*receivedObject), std::back_inserter(values));
            return std::make_shared<Array>(values);
        }

        Value mergeDictionaries(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

            if (!args[0].is<std::shared_ptr<Dictionary>>()) {
                throw std::runtime_error("expected `Dictionary` but got `" + args[0].type() + "`");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Dictionary>>();
            receivedObject->merge(*otherObject);
            return receivedObject;
        }

        Value insertDictionary(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Dictionary>>();

            if (!args[0].is<std::shared_ptr<Dictionary>>()) {
                throw std::runtime_error("expected `Dictionary` but got `" + args[0].type() + "`");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Dictionary>>();
            receivedObject->insert(otherObject->begin(), otherObject->end());
            return receivedObject;
        }

        Value removeDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
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

        Value createDictionaryFromKeys(Value receiver, const std::vector<Value> &args) {
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
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction>> table;
        table["len"] = std::make_shared<NativeFunction>("len", 0, getDictionaryLength);
        table["copy"] = std::make_shared<NativeFunction>("copy", 0, createDictionaryCopy);
        table["clear"] = std::make_shared<NativeFunction>("clear", 0, clearDictionary);
        table["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkDictionaryEmptiness);
        table["contains"] = std::make_shared<NativeFunction>("contains", 1, containsValue);
        table["get"] = std::make_shared<NativeFunction>("get", -1, getDictionaryElementByKey);
        table["setDefault"] = std::make_shared<NativeFunction>("setDefault", -1, setDictionaryElementByKey);
        table["keys"] = std::make_shared<NativeFunction>("keys", 0, retrieveDictionaryKeys);
        table["values"] = std::make_shared<NativeFunction>("values", 0, retrieveDictionaryValues);
        table["merge"] = std::make_shared<NativeFunction>("merge", 1, mergeDictionaries);
        table["insert"] = std::make_shared<NativeFunction>("insert", 1, insertDictionary);
        table["pop"] = std::make_shared<NativeFunction>("pop", 1, removeDictionaryElementByKey);
        table["fromKeys"] = std::make_shared<NativeFunction>("fromKeys", 2, createDictionaryFromKeys);
        return table;
    }
}