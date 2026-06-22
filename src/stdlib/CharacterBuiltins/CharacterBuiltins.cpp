#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

namespace stdlib::CharacterBuiltins {
    namespace {
        template<typename T>
        std::shared_ptr<NativeFunction> bind_method(const std::string &name, T &&function) {
            return std::make_shared<NativeFunction>(NativeFunction{
                .name = name,
                .arity = 0,
                .callable = [func = std::forward<T>(function)](const Value& receiver, const std::vector<Value> &args) {
                    return func(receiver.get<Character>());
                }
            });
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction> > methods_table;

        auto register_function = [&methods_table](std::string name, auto &&func) {
            methods_table.emplace(
                name,
                bind_method(name, std::forward<decltype(func)>(func))
            );
        };

        register_function("isAlpha", [](const Character &value) {
            return static_cast<bool>(std::isalpha(static_cast<unsigned char>(*value)));
        });

        register_function("isDigit", [](const Character &value) {
            return static_cast<bool>(std::isdigit(static_cast<unsigned char>(*value)));
        });

        register_function("isAlphaDigit", [](const Character &value) {
            return static_cast<bool>(std::isalnum(static_cast<unsigned char>(*value)));
        });

        register_function("isSpace", [](const Character &value) {
            return static_cast<bool>(std::isspace(static_cast<unsigned char>(*value)));
        });

        register_function("isUpper", [](const Character &value) {
            return static_cast<bool>(std::isupper(static_cast<unsigned char>(*value)));
        });

        register_function("isLower", [](const Character &value) {
            return static_cast<bool>(std::islower(static_cast<unsigned char>(*value)));
        });

        register_function("toUpper", [](const Character &value) {
            *value = static_cast<char>(std::toupper(static_cast<unsigned char>(*value)));
            return value;
        });

        register_function("toLower", [](const Character &value) {
            *value = static_cast<char>(std::tolower(static_cast<unsigned char>(*value)));
            return value;
        });

        return methods_table;
    }

}
