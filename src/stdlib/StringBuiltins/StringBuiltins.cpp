#include "stdlib/StringBuiltins/StringBuiltins.h"

namespace stdlib::StringBuiltins {
    namespace {
        Value retrieveStringLength(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return static_cast<double>(receivedObject.size());
        }

        Value copyString(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return receivedObject;
        }

        Value clearString(Value receiver, const std::vector<Value> &) {
            auto &receivedObject = receiver.get<std::string>();
            receivedObject.clear();
            return receivedObject;
        }

        Value checkStringEmptiness(Value receiver, const std::vector<Value> &) {
            return receiver.get<std::string>().empty();
        }

        Value reverseString(Value receiver, const std::vector<Value> &) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::reverse(receivedObject);
            return receivedObject;
        }

        Value retrieveCharacterByIndex(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<double>()) [[unlikely]] {
                throw std::invalid_argument("string index must be Number");
            }

            const auto &receivedObject = receiver.get<std::string>();

            const auto &index = static_cast<std::size_t>(args.front().get<double>());
            if (index >= receivedObject.size()) [[unlikely]] {
                throw std::out_of_range("string index out of bounds");
            }

            return receivedObject[index];
        }

        Value getIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("expected substring of type String");
            }

            const auto &receivedObject = receiver.get<std::string>();
            const auto &targetSubstring = args.front().get<std::string>();

            const auto it = receivedObject.find(targetSubstring);
            if (it == std::string::npos) return -1.0;
            return static_cast<double>(it);
        }

        Value getLastIndexOfSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("expected substring of type String");
            }

            const auto receivedObject = receiver.get<std::string>();
            const auto &targetSubstring = args.front().get<std::string>();

            const auto it = receivedObject.rfind(targetSubstring);
            if (it == std::string::npos) return -1.0;
            return static_cast<double>(it);
        }

        Value containsSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("expected substring of type String");
            }

            const auto &receivedObject = receiver.get<std::string>();
            return receivedObject.contains(args.front().get<std::string>());
        }

        Value starsWithSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("expected substring of type String");
            }

            const auto &receivedObject = receiver.get<std::string>();
            return receivedObject.starts_with(args.front().get<std::string>());
        }

        Value endsWithSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("expected substring of type String");
            }

            const auto &receivedObject = receiver.get<std::string>();
            return receivedObject.ends_with(args.front().get<std::string>());
        }

        Value takeSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<double>() || !args[1].is<double>()) [[unlikely]] {
                throw std::runtime_error("substring borders must be Number");
            }

            const auto &receivedObject = receiver.get<std::string>();

            const auto lhs = static_cast<std::size_t>(args.front().get<double>());
            const auto rhs = static_cast<std::size_t>(args[1].get<double>());
            if (lhs >= receivedObject.size()) [[unlikely]] {
                return "";
            }

            return receivedObject.substr(lhs, rhs);
        }

        Value convertUppercase(Value receiver, const std::vector<Value> &) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::transform(receivedObject, receivedObject.begin(), [](const unsigned char letter) {
                return std::toupper(letter);
            });
            return receivedObject;
        }

        Value convertLowercase(Value receiver, const std::vector<Value> &) {
            auto receivedObject = receiver.get<std::string>();
            std::ranges::transform(receivedObject, receivedObject.begin(), [](const unsigned char letter) {
                return std::tolower(letter);
            });
            return receivedObject;
        }

        Value trimString(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            const auto startingIndex = receivedObject.find_first_not_of(" \t\n\r\f\v");

            if (startingIndex == std::string::npos) [[unlikely]] {
                return "";
            }

            const auto endingIndex = receivedObject.find_last_not_of(" \t\n\r\f\v");
            return receivedObject.substr(startingIndex, endingIndex - startingIndex + 1);
        }

        Value replaceSubstring(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>() && !args[1].is<std::string>()) [[unlikely]] {
                throw std::runtime_error("expected substring of type String");
            }

            auto receivedObject = receiver.get<std::string>();

            const auto &targetSubstring = args.front().get<std::string>();
            const auto &replacementSubstring = args[1].get<std::string>();

            if (targetSubstring.empty()) return receivedObject;

            if (const auto it = receivedObject.find(targetSubstring); it != std::string::npos) {
                receivedObject.replace(it, targetSubstring.length(), replacementSubstring);
            }

            return receivedObject;
        }

        Value replaceAllSubstrings(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::string>() && !args[1].is<std::string>()) [[unlikely]] {
                throw std::runtime_error("expected substring of type String");
            }

            auto receivedObject = receiver.get<std::string>();
            const auto &targetSubstring = args.front().get<std::string>();
            const auto &replacementSubstring = args[1].get<std::string>();

            if (targetSubstring.empty()) [[unlikely]] {
                return receivedObject;
            }

            std::size_t it = 0;
            while ((it = receivedObject.find(targetSubstring, it)) != std::string::npos) {
                receivedObject.replace(it, targetSubstring.length(), replacementSubstring);
                it += replacementSubstring.length();
            }

            return receivedObject;
        }

        Value checkIfAlpha(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](const unsigned char letter) {
                return std::isalpha(letter);
            });
        }

        Value checkIfNumeric(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](const unsigned char letter) {
                return std::isdigit(letter);
            });
        }

        Value checkIfAlphaNumeric(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](const unsigned char letter) {
                return std::isalnum(letter);
            });
        }

        Value checkIfWhitespace(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::string>();
            return !receivedObject.empty() && std::ranges::all_of(receivedObject, [](const unsigned char letter) {
                return std::isspace(letter);
            });
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {
                "len", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "len",
                    .arity = 0,
                    .callable = retrieveStringLength
                })
            },
            {
                "copy", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "copy",
                    .arity = 0,
                    .callable = copyString
                })
            },
            {
                "clear", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "clear",
                    .arity = 0,
                    .callable = clearString
                })
            },
            {
                "isEmpty", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isEmpty",
                    .arity = 0,
                    .callable = checkStringEmptiness
                })
            },
            {
                "reverse", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "reverse",
                    .arity = 0,
                    .callable = reverseString
                })
            },
            {
                "at", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "at",
                    .arity = 1,
                    .callable = retrieveCharacterByIndex
                })
            },
            {
                "indexOf", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "indexOf",
                    .arity = 1,
                    .callable = getIndexOfSubstring
                })
            },
            {
                "lastIndexOf", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "lastIndexOf",
                    .arity = 1,
                    .callable = getLastIndexOfSubstring
                })
            },
            {
                "contains", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "contains",
                    .arity = 1,
                    .callable = containsSubstring
                })
            },
            {
                "startsWith", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "startsWith",
                    .arity = 1,
                    .callable = starsWithSubstring
                })
            },
            {
                "endsWith", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "endsWith",
                    .arity = 1,
                    .callable = endsWithSubstring
                })
            },
            {
                "substring", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "substring",
                    .arity = 2,
                    .callable = takeSubstring
                })
            },
            {
                "toLower", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "toLower",
                    .arity = 0,
                    .callable = convertLowercase
                })
            },

            {
                "toUpper", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "toUpper",
                    .arity = 0,
                    .callable = convertUppercase
                })
            },
            {
                "trim", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "trim",
                    .arity = 0,
                    .callable = trimString
                })
            },
            {
                "replace", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "replace",
                    .arity = 2,
                    .callable = replaceSubstring
                })
            },
            {
                "replaceAll", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "replaceAll",
                    .arity = 2,
                    .callable = replaceAllSubstrings
                })
            },
            {
                "isAlpha", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isAlpha",
                    .arity = 0,
                    .callable = checkIfAlpha
                })
            },
            {
                "isNumeric", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isNumeric",
                    .arity = 0,
                    .callable = checkIfNumeric
                })
            },
            {
                "isAlphaNumeric", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isAlphaNumeric",
                    .arity = 0,
                    .callable = checkIfAlphaNumeric
                })
            },
            {
                "isWhitespace", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isWhitespace",
                    .arity = 0,
                    .callable = checkIfWhitespace
                })
            },
        };
    }
}
