#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

namespace stdlib::CharacterBuiltins {
    namespace {
        template<typename T>
        std::shared_ptr<NativeFunction> make_zero_arity_function(const std::string &name, T &&function) {
            return std::make_shared<NativeFunction>(NativeFunction{
                .name = name,
                .arity = 0,
                .callable = [func = std::forward<T>(function)](const Value& receiver, const std::vector<Value> &args) {
                    return func(receiver.get<char>());
                }
            });
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction> > methods_table;

        auto register_zero_arity_function = [&methods_table](std::string name, auto &&func) {
            methods_table.emplace(
                name,
                make_zero_arity_function(name, std::forward<decltype(func)>(func))
            );
        };

        register_zero_arity_function("isAlpha", [](const char &value) {
            return static_cast<bool>(std::isalpha(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("isNumeric", [](const char &value) {
            return static_cast<bool>(std::isdigit(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("isAlphaNumeric", [](const char &value) {
            return static_cast<bool>(std::isalnum(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("isWhitespace", [](const char &value) {
            return static_cast<bool>(std::isspace(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("isUppercase", [](const char &value) {
            return static_cast<bool>(std::isupper(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("isLowercase", [](const char &value) {
            return static_cast<bool>(std::islower(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("toUpper", [](const char &value) {
            return static_cast<bool>(std::toupper(static_cast<unsigned char>(value)));
        });

        register_zero_arity_function("toLower", [](const char &value) {
            return static_cast<bool>(std::tolower(static_cast<unsigned char>(value)));
        });

        return methods_table;
    }

}
