#pragma once

#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"
#include "stdlib/SetBuiltins/SetBuiltins.h"
#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"
#include "stdlib/StringBuiltins/StringBuiltins.h"
#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"
#include "stdlib/StandardIOBuiltins/StandardIOBuiltins.h"
#include "stdlib/MathsBuiltins/MathsBuiltins.h"
#include "stdlib/FileBuiltins/FileBuiltins.h"
#include "stdlib/TypesBuiltins/TypesBuiltins.h"

class StdLib {
public:
    StdLib();

    [[nodiscard]] std::shared_ptr<NativeFunction> get_array_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_set_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_dictionary_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_string_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_file_method(const std::string &name);
    [[nodiscard]] std::shared_ptr<NativeFunction> get_character_method(const std::string &name);
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& get_standard_io_methods() const;
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& get_maths_methods() const;
    [[nodiscard]] const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& get_types_methods() const;
private:
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_array_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_set_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_dictionary_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_string_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_file_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_character_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_io_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_maths_methods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> std_types_methods;
};