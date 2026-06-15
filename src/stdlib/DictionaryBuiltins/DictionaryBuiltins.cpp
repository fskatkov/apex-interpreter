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

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, getDictionaryLength);
        registry["copy"] = std::make_shared<NativeFunction>("copy", 0, createDictionaryCopy);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearDictionary);
    }
}