#include "stdlib/StringBuiltins/StringBuiltins.h"

namespace stdlib::StringBuiltins {
    namespace {
        Value retrieveStringLength(Value receiver, const std::vector<Value> &args) {
            return static_cast<double>(receiver.get<std::string>().size());
        }

        Value checkStringEmptiness(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::string>().empty();
        }

        Value reverseString(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::reverse(receivedObject);
            return receivedObject;
        }

        Value retrieveCharacterByIndex(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::string>();
            if (!args[0].is<double>()) {
                throw std::runtime_error("string index must be number");
            }
            return receivedObject[static_cast<std::size_t>(args[0].get<double>())];
        }

        Value getIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
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

        Value getLastIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
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

        Value containsSubstring(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::string>();
            if (!args[0].is<std::string>()) {
                throw std::runtime_error("expected substring of type String");
            }
            return receivedObject.find(args[0].get<std::string>()) != std::string::npos;
        }

        Value starsWithSubstring(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::string>();
            if (!args[0].is<std::string>()) {
                throw std::runtime_error("expected substring of type String");
            }
            return receivedObject.starts_with(args[0].get<std::string>());
        }

        Value endsWithSubstring(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::string>();
            if (!args[0].is<std::string>()) {
                throw std::runtime_error("expected substring of type String");
            }
            return receivedObject.ends_with(args[0].get<std::string>());
        }

        Value takeSubstring(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::string>();
            if (!args[0].is<double>() && !args[1].is<double>()) {
                throw std::runtime_error("substring borders must be numbers");
            }

            const auto lhs = static_cast<std::size_t>(args[0].get<double>());
            const auto rhs = static_cast<std::size_t>(args[1].get<double>());
            return receivedObject.substr(lhs, rhs);
        }

        Value convertLowercase(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::transform(receivedObject, receivedObject.begin(), [](unsigned char letter) {
                return std::tolower(letter);
            });
            return receivedObject;
        }

        Value convertUppercase(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::transform(receivedObject, receivedObject.begin(), [](unsigned char letter) {
                return std::toupper(letter);
            });
            return receivedObject;
        }

        Value trimString(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            receivedObject.erase(0, receivedObject.find_first_not_of(" \t\n\r\f\v"));
            receivedObject.erase(receivedObject.find_last_not_of(" \t\n\r\f\v") + 1);
            return receivedObject;
        }

        Value replaceSubstring(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();

            if (!args[0].is<std::string>() && !args[1].is<std::string>()) {
                throw std::runtime_error("expected substring of type String");
            }

            const auto targetSubstring = args[0].get<std::string>();
            const auto replacementSubstring = args[1].get<std::string>();

            if (auto it = receivedObject.find(targetSubstring); it != std::string::npos) {
                receivedObject.replace(it, targetSubstring.length(), replacementSubstring);
            }

            return receivedObject;
        }

        Value replaceAllSubstrings(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();

            if (!args[0].is<std::string>() && !args[1].is<std::string>()) {
                throw std::runtime_error("expected substring of type String");
            }

            const auto targetSubstring = args[0].get<std::string>();
            const auto replacementSubstring = args[1].get<std::string>();

            std::size_t it = 0;
            while ((it = receivedObject.find(targetSubstring, it)) != std::string::npos) {
                receivedObject.replace(it, targetSubstring.length(), replacementSubstring);
                it += replacementSubstring.length();
            }

            return receivedObject;
        }

        Value checkIfAlpha(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](unsigned char letter) {
                return std::isalpha(letter);
            });
        }

        Value checkIfNumeric(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](unsigned char letter) {
                return std::isdigit(letter);
            });
        }

        Value checkIfAlphaNumeric(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](unsigned char letter) {
                return std::isalnum(letter);
            });
        }

        Value checkIfWhitespace(Value receiver, const std::vector<Value> &args) {
            auto receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](unsigned char letter) {
                return std::isspace(letter);
            });
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction>> table;
        table["len"] = std::make_shared<NativeFunction>("len", 0, retrieveStringLength);
        table["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkStringEmptiness);
        table["reverse"] = std::make_shared<NativeFunction>("reverse", 0, reverseString);
        table["at"] = std::make_shared<NativeFunction>("at", 1, retrieveCharacterByIndex);
        table["indexOf"] = std::make_shared<NativeFunction>("indexOf", 1, getIndexOfSubstring);
        table["lastIndexOf"] = std::make_shared<NativeFunction>("lastIndexOf", 1, getLastIndexOfSubstring);
        table["contains"] = std::make_shared<NativeFunction>("contains", 1, containsSubstring);
        table["startsWith"] = std::make_shared<NativeFunction>("startsWith", 1, starsWithSubstring);
        table["endsWith"] = std::make_shared<NativeFunction>("endsWith", 1, endsWithSubstring);
        table["substring"] = std::make_shared<NativeFunction>("substring", 2, takeSubstring);
        table["lower"] = std::make_shared<NativeFunction>("lower", 0, convertLowercase);
        table["upper"] = std::make_shared<NativeFunction>("upper", 0, convertUppercase);
        table["trim"] = std::make_shared<NativeFunction>("trim", 0, trimString);
        table["replace"] = std::make_shared<NativeFunction>("replace", 2, replaceSubstring);
        table["replaceAll"] = std::make_shared<NativeFunction>("replaceAll", 2, replaceAllSubstrings);
        table["isAlpha"] = std::make_shared<NativeFunction>("isAlpha", 0, checkIfAlpha);
        table["isNumeric"] = std::make_shared<NativeFunction>("isNumeric", 0, checkIfNumeric);
        table["isAlphaNumeric"] = std::make_shared<NativeFunction>("isAlphaNumeric", 0, checkIfAlphaNumeric);
        table["isWhitespace"] = std::make_shared<NativeFunction>("isWhitespace", 0, checkIfWhitespace);
        return table;
    }
}
