#pragma once

#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"
#include "stdlib/SetBuiltins/SetBuiltins.h"
#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"
#include "stdlib/StringBuiltins/StringBuiltins.h"
#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"
#include "stdlib/StandardIOBuiltins/StandardIOBuiltins.h"

class StdLib {
public:
    StdLib();

    [[nodiscard]] std::shared_ptr<NativeFunction> get_array_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_set_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_dictionary_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_string_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_character_method(const std::string &name);
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& get_standard_io_methods() const;
private:
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> arrayMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> setMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> dictionaryMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> stringMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> characterMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> standardIOMethods;
};