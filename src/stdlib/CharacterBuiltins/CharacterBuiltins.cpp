#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

namespace stdlib::CharacterBuiltins {
    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "isAlpha", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isAlpha",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::isalpha(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "isNumeric", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isNumeric",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::isdigit(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "isAlphaNumeric", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isAlphaNumeric",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::isalnum(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "isWhitespace", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isWhitespace",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::isspace(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "isUppercase", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isUppercase",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::isupper(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "isLowercase", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isLowercase",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<bool>(std::islower(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "toUpper", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "toUpper",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<char>(std::toupper(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
            {
                "toUpper", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "toUpper",
                    .arity = 0,
                    .callable = [](Value receiver, const std::vector<Value> &) {
                        return static_cast<char>(std::tolower(static_cast<unsigned char>(receiver.get<char>())));
                    }
                })
            },
        };
    }
}
