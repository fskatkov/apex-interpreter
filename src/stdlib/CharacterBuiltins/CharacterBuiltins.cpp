#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

namespace stdlib::CharacterBuiltins {
    namespace {
        Value checkIfAlpha(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::isalpha(static_cast<unsigned char>(receivedObject)));
        }

        Value checkIfNumeric(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::isdigit(static_cast<unsigned char>(receivedObject)));
        }

        Value checkIfAlphaNumeric(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::isalnum(static_cast<unsigned char>(receivedObject)));
        }

        Value checkIfWhitespace(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::isspace(static_cast<unsigned char>(receivedObject)));
        }

        Value checkIfUppercase(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::isupper(static_cast<unsigned char>(receivedObject)));
        }

        Value checkIfLowercase(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<bool>(std::islower(static_cast<unsigned char>(receivedObject)));
        }

        Value convertUppercase(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<char>(std::toupper(static_cast<unsigned char>(receivedObject)));
        }

        Value convertLowercase(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<char>();
            return static_cast<char>(std::tolower(static_cast<unsigned char>(receivedObject)));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction>> table;
        table["isAlpha"] = std::make_shared<NativeFunction>("isAlpha", 0, checkIfAlpha);
        table["isNumeric"] = std::make_shared<NativeFunction>("isNumeric", 0, checkIfNumeric);
        table["isAlphaNumeric"] = std::make_shared<NativeFunction>("isAlphaNumeric", 0, checkIfAlphaNumeric);
        table["isWhitespace"] = std::make_shared<NativeFunction>("isWhitespace", 0, checkIfWhitespace);
        table["isUppercase"] = std::make_shared<NativeFunction>("isUppercase", 0, checkIfUppercase);
        table["isLowercase"] = std::make_shared<NativeFunction>("isLowercase", 0, checkIfLowercase);
        table["upper"] = std::make_shared<NativeFunction>("upper", 0, convertUppercase);
        table["lower"] = std::make_shared<NativeFunction>("lower", 0, convertLowercase);
        return table;
    }
}
