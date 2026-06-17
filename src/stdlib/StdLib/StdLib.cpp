#include "stdlib/StdLib/StdLib.h"

void StdLib::registerStandardLibrary() {
    stdlib::ArrayBuiltins::registerMethods(arrayMethods);
    stdlib::SetBuiltins::registerMethods(setMethods);
    stdlib::DictionaryBuiltins::registerMethods(dictionaryMethods);
    stdlib::StringBuiltins::registerMethods(stringMethods);
    stdlib::CharacterBuiltins::registerMethods(characterMethods);
}

std::shared_ptr<NativeFunction> StdLib::getArrayMethod(const std::string &name) {
    if (arrayMethods.contains(name)) return arrayMethods[name];
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getSetMethod(const std::string &name) {
    if (setMethods.contains(name)) return setMethods[name];
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getDictionaryMethod(const std::string &name) {
    if (dictionaryMethods.contains(name)) return dictionaryMethods[name];
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getStringMethod(const std::string &name) {
    if (stringMethods.contains(name)) return stringMethods[name];
    return nullptr;
}

std::shared_ptr<NativeFunction> StdLib::getCharacterMethod(const std::string &name) {
    if (characterMethods.contains(name)) return characterMethods[name];
    return nullptr;
}
