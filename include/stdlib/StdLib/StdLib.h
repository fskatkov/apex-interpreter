#pragma once

#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"
#include "stdlib/SetBuiltins/SetBuiltins.h"
#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"
#include "stdlib/StringBuiltins/StringBuiltins.h"
#include "stdlib/CharacterBuiltins/CharacterBuiltins.h"

struct StdLib {
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> arrayMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> setMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> dictionaryMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> stringMethods;
    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> characterMethods;

    void registerStandardLibrary();
    [[maybe_unused]] std::shared_ptr<NativeFunction> getArrayMethod(const std::string &name);
    [[maybe_unused]] std::shared_ptr<NativeFunction> getSetMethod(const std::string &name);
    [[maybe_unused]] std::shared_ptr<NativeFunction> getDictionaryMethod(const std::string &name);
    [[maybe_unused]] std::shared_ptr<NativeFunction> getStringMethod(const std::string &name);
    [[maybe_unused]] std::shared_ptr<NativeFunction> getCharacterMethod(const std::string &name);
};
