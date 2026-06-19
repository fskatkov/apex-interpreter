#include "stdlib/StdLib/StdLib.h"

StdLib::StdLib()
    : std_array_methods(stdlib::ArrayBuiltins::registerMethods()),
      std_set_methods(stdlib::SetBuiltins::registerMethods()),
      std_dictionary_methods(stdlib::DictionaryBuiltins::registerMethods()),
      std_string_methods(stdlib::StringBuiltins::registerMethods()),
      std_character_methods(stdlib::CharacterBuiltins::registerMethods()),
      std_io_methods(stdlib::StandardIOBuiltins::registerMethods()),
      std_maths_methods(stdlib::MathsBuiltins::registerMethods()) {
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

std::shared_ptr<NativeFunction> StdLib::get_character_method(const std::string &name) {
    if (const auto it = std_character_methods.find(name); it != std_character_methods.end()) return it->second;
    return nullptr;
}

const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& StdLib::get_standard_io_methods() const {
    return std_io_methods;
}

const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& StdLib::get_maths_methods() const {
    return std_maths_methods;
}
