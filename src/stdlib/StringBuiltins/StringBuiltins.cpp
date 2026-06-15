#include "stdlib/StringBuiltins/StringBuiltins.h"

namespace stdlib::StringBuiltins {
    static Value retrieveStringLength(Value receiver, const std::vector<Value> &args) {
        return static_cast<double>(receiver.get<std::string>().size());
    }

    static Value checkStringEmptiness(Value receiver, const std::vector<Value> &args) {
        return receiver.get<std::string>().empty();
    }

    static Value reverseString(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<std::string>();
        std::ranges::reverse(receivedObject);
        return receivedObject;
    }

    static Value retrieveCharacterByIndex(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<double>()) {
            throw std::runtime_error("string index must be number");
        }
        return receivedObject[static_cast<std::size_t>(args[0].get<double>())];
    }

    static Value getIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<std::string>()) {
            throw std::runtime_error("expected substring of type String");
        }

        const auto it = receivedObject.find(args[0].get<std::string>());
        if (it == std::string::npos) {
            return -1.0;
        }
        return static_cast<double>(it);
    }

    static Value getLastIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<std::string>()) {
            throw std::runtime_error("expected substring of type String");
        }

        const auto it = receivedObject.rfind(args[0].get<std::string>());
        if (it == std::string::npos) {
            return -1.0;
        }
        return static_cast<double>(it);
    }

    static Value containsSubstring(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<std::string>()) {
            throw std::runtime_error("expected substring of type String");
        }
        return receivedObject.find(args[0].get<std::string>()) != std::string::npos;
    }

    static Value starsWithSubstring(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<std::string>()) {
            throw std::runtime_error("expected substring of type String");
        }
        return receivedObject.starts_with(args[0].get<std::string>());
    }

    static Value endsWithSubstring(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::string>();
        if (!args[0].is<std::string>()) {
            throw std::runtime_error("expected substring of type String");
        }
        return receivedObject.ends_with(args[0].get<std::string>());
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, retrieveStringLength);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkStringEmptiness);
        registry["reverse"] = std::make_shared<NativeFunction>("reverse", 0, reverseString);

        registry["at"] = std::make_shared<NativeFunction>("at", 1, retrieveCharacterByIndex);
        registry["indexOf"] = std::make_shared<NativeFunction>("indexOf", 1, getIndexOfSubstring);
        registry["lastIndexOf"] = std::make_shared<NativeFunction>("lastIndexOf", 1, getLastIndexOfSubstring);
        registry["contains"] = std::make_shared<NativeFunction>("contains", 1, containsSubstring);
        registry["startsWith"] = std::make_shared<NativeFunction>("startsWith", 1, starsWithSubstring);
        registry["endsWith"] = std::make_shared<NativeFunction>("endsWith", 1, endsWithSubstring);
    }
}
