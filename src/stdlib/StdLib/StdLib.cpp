#include "stdlib/StdLib/StdLib.h"

StdLib::StdLib()
    : arrayMethods(stdlib::ArrayBuiltins::registerMethods()),
      setMethods(stdlib::SetBuiltins::registerMethods()),
      dictionaryMethods(stdlib::DictionaryBuiltins::registerMethods()),
      stringMethods(stdlib::StringBuiltins::registerMethods()),
      characterMethods(stdlib::CharacterBuiltins::registerMethods()),
      standardIOMethods(stdlib::StandardIOBuiltins::registerMethods()) {
}

std::shared_ptr<NativeFunction> StdLib::get_array_method(const std::string &name) {
    if (const auto it = arrayMethods.find(name); it != arrayMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_set_method(const std::string &name) {
    if (const auto it = setMethods.find(name); it != setMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_dictionary_method(const std::string &name) {
    if (const auto it = dictionaryMethods.find(name); it != dictionaryMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_string_method(const std::string &name) {
    if (const auto it = stringMethods.find(name); it != stringMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::get_character_method(const std::string &name) {
    if (const auto it = characterMethods.find(name); it != characterMethods.end()) return it->second;
    return nullptr;
}

const std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& StdLib::get_standard_io_methods() const {
    return standardIOMethods;
}