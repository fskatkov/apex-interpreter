#include "stdlib/StdLib/StdLib.h"

StdLib::StdLib()
    : std_array_methods(stdlib::ArrayBuiltins::register_methods()),
      std_set_methods(stdlib::SetBuiltins::register_methods()),
      std_dictionary_methods(stdlib::DictionaryBuiltins::register_methods()),
      std_string_methods(stdlib::StringBuiltins::register_methods()),
      std_file_methods(stdlib::FileBuiltins::register_methods()),
      std_character_methods(stdlib::CharacterBuiltins::register_methods()) {
    stdlib_functions.merge(stdlib::UtilitiesBuiltins::register_methods());
    stdlib_functions.merge(stdlib::MathsBuiltins::register_methods());
    stdlib_functions.merge(stdlib::TypesBuiltins::register_methods());
    stdlib_functions.merge(stdlib::OSBuiltins::register_methods());
}

std::shared_ptr<NativeFunction> StdLib::get_array_method(const std::string &name) {
    if (const auto it = std_array_methods.find(name); it != std_array_methods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_set_method(const std::string &name) {
    if (const auto it = std_set_methods.find(name); it != std_set_methods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_dictionary_method(const std::string &name) {
    if (const auto it = std_dictionary_methods.find(name); it != std_dictionary_methods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_string_method(const std::string &name) {
    if (const auto it = std_string_methods.find(name); it != std_string_methods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_file_method(const std::string &name) {
    if (const auto it = std_file_methods.find(name); it != std_file_methods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_character_method(const std::string &name) {
    if (const auto it = std_character_methods.find(name); it != std_character_methods.end()) return it->second;
    return nullptr;
}

void StdLib::get_stdlib_functions(std::unordered_map<std::string, Value> &global_variables) {
    for (const auto &[name, native_function]: stdlib_functions) {
        global_variables[name] = native_function;
    }
}
