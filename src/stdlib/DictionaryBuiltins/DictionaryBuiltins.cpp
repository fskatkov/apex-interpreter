#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"

namespace stdlib::DictionaryBuiltins {
    namespace {
        Value getDictionaryLength(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            return static_cast<double>(receivedObject->size());
        }

        Value createDictionaryCopy(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            return std::make_shared<Dictionary>(*receivedObject);
        }

        Value clearDictionary(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            receivedObject->clear();
            return receivedObject;
        }

        Value checkDictionaryEmptiness(Value receiver, const std::vector<Value> &) {
            return receiver.get<std::shared_ptr<Dictionary> >()->empty();
        }

        Value containsValue(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<double>() && !args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("dictionary key must be Number or String");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            return receivedObject->contains(args.front());
        }

        Value getDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
            if (args.empty()) [[unlikely]] {
                throw std::invalid_argument("get requires at least one element");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            const auto &defaultValue = args.size() > 1 ? args[1] : NIL{};

            if (!args.front().is<double>() && !args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("dictionary key must be Number or String");
            }

            if (const auto it = receivedObject->find(args.front()); it != receivedObject->end()) {
                return it->second;
            }

            return defaultValue;
        }

        Value setDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
            if (args.empty()) [[unlikely]] {
                throw std::invalid_argument("setDefault requires at least one element");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();

            if (!args.front().is<double>() && !args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("dictionary key must be Number or String");
            }

            const auto &key = args.front();
            const auto &defaultValue = args.size() > 1 ? args[1] : NIL{};

            auto [it, insertedElement] = receivedObject->try_emplace(key, defaultValue);
            return it->second;
        }

        Value retrieveDictionaryKeys(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            return std::make_shared<Array>(std::views::keys(*receivedObject) | std::ranges::to<Array>());
        }

        Value retrieveDictionaryValues(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();
            return std::make_shared<Array>(std::views::values(*receivedObject) | std::ranges::to<Array>());
        }

        Value mergeDictionaries(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Dictionary> >()) [[unlikely]] {
                throw std::invalid_argument(std::format("expected Dictionary but got {}", args.front().type()));
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();

            const auto &otherObject = args.front().get<std::shared_ptr<Dictionary> >();

            for (const auto &[key, value]: *otherObject) {
                receivedObject->insert_or_assign(key, value);
            }

            return receivedObject;
        }

        Value insertDictionary(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Dictionary> >()) [[unlikely]] {
                throw std::invalid_argument(std::format("expected Dictionary but got {}", args.front().type()));
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();

            const auto otherObject = args.front().get<std::shared_ptr<Dictionary> >();
            receivedObject->insert(otherObject->begin(), otherObject->end());
            return receivedObject;
        }

        Value removeDictionaryElementByKey(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<double>() && !args.front().is<std::string>()) [[unlikely]] {
                throw std::invalid_argument("dictionary key must be Number or String");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Dictionary> >();

            if (auto it = receivedObject->find(args.front()); it != receivedObject->end()) {
                auto extractedValue = std::move(it->second);
                receivedObject->erase(it);
                return extractedValue;
            }

            throw std::runtime_error("dictionary element does not exist");
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "len", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "len",
                    .arity = 0,
                    .callable = getDictionaryLength
                })
            },
            {
                "clear", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "clear",
                    .arity = 0,
                    .callable = clearDictionary
                })
            },
            {
                "copy", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "copy",
                    .arity = 0,
                    .callable = createDictionaryCopy
                })
            },
            {
                "isEmpty", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isEmpty",
                    .arity = 0,
                    .callable = checkDictionaryEmptiness
                })
            },
            {
                "insert", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "insert",
                    .arity = 1,
                    .callable = insertDictionary
                })
            },
            {
                "remove", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "remove",
                    .arity = 1,
                    .callable = removeDictionaryElementByKey
                })
            },
            {
                "contains", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "contains",
                    .arity = 1,
                    .callable = containsValue
                })
            },
            {
                "get", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "get",
                    .arity = -1,
                    .callable = getDictionaryElementByKey
                })
            },
            {
                "setDefault", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "setDefault",
                    .arity = -1,
                    .callable = setDictionaryElementByKey
                })
            },
            {
                "keys", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "keys",
                    .arity = 0,
                    .callable = retrieveDictionaryKeys
                })
            },
            {
                "values", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "values",
                    .arity = 0,
                    .callable = retrieveDictionaryValues
                })
            },
            {
                "merge", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "merge",
                    .arity = 1,
                    .callable = mergeDictionaries
                })
            }
        };
    }
}
