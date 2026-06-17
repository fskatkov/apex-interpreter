#include "stdlib/StdLib/StdLib.h"

StdLib::StdLib() {
    arrayMethods = stdlib::ArrayBuiltins::registerMethods();
    setMethods = stdlib::SetBuiltins::registerMethods();
    dictionaryMethods = stdlib::DictionaryBuiltins::registerMethods();
    stringMethods = stdlib::StringBuiltins::registerMethods();
    characterMethods = stdlib::CharacterBuiltins::registerMethods();
}

std::shared_ptr<NativeFunction> StdLib::getArrayMethod(const std::string &name) {
    if (const auto it = arrayMethods.find(name); it != arrayMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getSetMethod(const std::string &name) {
    if (const auto it = setMethods.find(name); it != setMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getDictionaryMethod(const std::string &name) {
    if (const auto it = dictionaryMethods.find(name); it != dictionaryMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getStringMethod(const std::string &name) {
    if (const auto it = stringMethods.find(name); it != stringMethods.end()) return it->second;
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getCharacterMethod(const std::string &name) {
    if (const auto it = characterMethods.find(name); it != characterMethods.end()) return it->second;
    return nullptr;
}
