#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

namespace stdlib::CharacterBuiltins {
    static Value checkIfAlpha(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::isalpha(static_cast<unsigned char>(receivedObject)));
    }

    static Value checkIfNumeric(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(receivedObject)));
    }

    static Value checkIfAlphaNumeric(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::isalnum(static_cast<unsigned char>(receivedObject)));
    }

    static Value checkIfWhitespace(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::isspace(static_cast<unsigned char>(receivedObject)));
    }

    static Value checkIfUppercase(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::isupper(static_cast<unsigned char>(receivedObject)));
    }

    static Value checkIfLowercase(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<bool>(std::islower(static_cast<unsigned char>(receivedObject)));
    }

    static Value convertUppercase(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<char>(std::toupper(static_cast<unsigned char>(receivedObject)));
    }

    static Value convertLowercase(Value receiver, const std::vector<Value> &args) {
        auto receivedObject = receiver.get<char>();
        return static_cast<char>(std::tolower(static_cast<unsigned char>(receivedObject)));
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["isAlpha"] = std::make_shared<NativeFunction>("isAlpha", 0, checkIfAlpha);
        registry["isNumeric"] = std::make_shared<NativeFunction>("isNumeric", 0, checkIfNumeric);
        registry["isAlphaNumeric"] = std::make_shared<NativeFunction>("isAlphaNumeric", 0, checkIfAlphaNumeric);
        registry["isWhitespace"] = std::make_shared<NativeFunction>("isWhitespace", 0, checkIfWhitespace);
        registry["isUppercase"] = std::make_shared<NativeFunction>("isUppercase", 0, checkIfUppercase);
        registry["isLowercase"] = std::make_shared<NativeFunction>("isLowercase", 0, checkIfLowercase);
        registry["upper"] = std::make_shared<NativeFunction>("upper", 0, convertUppercase);
        registry["lower"] = std::make_shared<NativeFunction>("lower", 0, convertLowercase);
    }
}
