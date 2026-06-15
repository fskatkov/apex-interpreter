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

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["len"] = std::make_shared<NativeFunction>("len", 0, retrieveStringLength);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkStringEmptiness);
        registry["reverse"] = std::make_shared<NativeFunction>("reverse", 0, reverseString);
    }
}
