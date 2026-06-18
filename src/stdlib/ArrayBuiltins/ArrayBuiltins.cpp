#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"

namespace stdlib::ArrayBuiltins {
    namespace {
        Value retrieveArraySize(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return static_cast<double>(receivedObject->size());
        }

        Value clearArray(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            receivedObject->clear();
            return receivedObject;
        }

        Value createArrayClone(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return std::make_shared<Array>(*receivedObject);
        }

        Value reverseArray(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            std::ranges::reverse(*receivedObject);
            return NIL{};
        }

        Value checkArrayEmptiness(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return receivedObject->empty();
        }

        Value getFrontArrayElement(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the front element");
            }
            return receivedObject->front();
        }

        Value getBackArrayElement(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the back element");
            }
            return receivedObject->back();
        }

        Value getElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args.front().get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            return (*receivedObject)[idx];
        }

        Value updateElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args.front().get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            (*receivedObject)[idx] = args[1];
            return receivedObject;
        }

        Value appendElementToArrayEnd(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            receivedObject->push_back(args.front());
            return NIL{};
        }

        Value appendElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args.front().get<double>());
            if (idx > receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            receivedObject->insert(receivedObject->begin() + idx, args[1]);
            return NIL{};
        }

        Value removeElementFromArrayEnd(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (receivedObject->empty()) [[unlikely]] {
                throw std::runtime_error("cannot pop from an empty array");
            }

            auto backElement = std::move(receivedObject->back());
            receivedObject->pop_back();
            return backElement;
        }

        Value removeElementByIndex(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            const auto idx = static_cast<std::size_t>(args.front().get<double>());
            if (idx >= receivedObject->size()) [[unlikely]] {
                throw std::out_of_range("array index out of bounds");
            }

            receivedObject->erase(receivedObject->begin() + idx);
            return NIL{};
        }

        Value sliceArray(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<double>() || !args[1].is<double>()) [[unlikely]] {
                throw std::invalid_argument("array index must be a number");
            }

            if (args.front().get<double>() < 0 || args[1].get<double>() < 0) [[unlikely]] {
                throw std::out_of_range("array indices must be non-negative");
            }

            const auto startingIndex = static_cast<std::size_t>(args.front().get<double>());
            const auto finalIndex = static_cast<std::size_t>(args[1].get<double>());

            if (startingIndex >= receivedObject->size() || startingIndex > finalIndex) [[unlikely]] {
                return std::make_shared<Array>();
            }

            const auto resultingIndex = std::min(receivedObject->size() - 1, finalIndex);

            return std::make_shared<Array>(
                receivedObject->begin() + startingIndex,
                receivedObject->begin() + resultingIndex + 1
            );
        }

        Value concatenateTwoArrays(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            if (!args.front().is<std::shared_ptr<Array> >()) [[unlikely]] {
                throw std::invalid_argument(std::format("cannot concatenate array and {}", args.front().type()));
            }

            const auto &anotherArray = args.front().get<std::shared_ptr<Array> >();
            receivedObject->reserve(receivedObject->size() + anotherArray->size());
            receivedObject->insert(receivedObject->end(), anotherArray->begin(), anotherArray->end());
            return NIL{};
        }

        Value retrieveFirstIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            const auto it = std::ranges::find(*receivedObject, args.front());
            if (it == receivedObject->end()) [[unlikely]] {
                throw std::runtime_error(std::format("not found element {}", it->str()));
            }

            return static_cast<double>(std::ranges::distance(receivedObject->begin(), it));
        }

        Value retrieveLastIndexOfArrayElement(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();

            const auto it = std::ranges::find(std::ranges::rbegin(*receivedObject), std::ranges::rend(*receivedObject),
                                              args.front());
            if (it == receivedObject->rend()) {
                throw std::runtime_error(std::format("not found element {}", it->str()));
            }

            const auto index = receivedObject->size() - 1 - std::ranges::distance(
                                   std::ranges::rbegin(*receivedObject), it);
            return static_cast<double>(index);
        }

        Value containsValue(Value receiver, const std::vector<Value> &args) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Array> >();
            return std::ranges::contains(*receivedObject, args.front());
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "len", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "len",
                    .arity = 0,
                    .callable = retrieveArraySize
                })
            },
            {
                "clear", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "clear",
                    .arity = 0,
                    .callable = clearArray
                })
            },
            {
                "copy", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "copy",
                    .arity = 0,
                    .callable = createArrayClone
                })
            },
            {
                "reverse", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "reverse",
                    .arity = 0,
                    .callable = reverseArray
                })
            },
            {
                "isEmpty", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isEmpty",
                    .arity = 0,
                    .callable = checkArrayEmptiness
                })
            },
            {
                "first", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "first",
                    .arity = 0,
                    .callable = getFrontArrayElement
                })
            },
            {
                "last", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "last",
                    .arity = 0,
                    .callable = getBackArrayElement
                })
            },
            {
                "at", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "at",
                    .arity = 1,
                    .callable = getElementByIndex
                })
            },
            {
                "set", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "set",
                    .arity = 2,
                    .callable = updateElementByIndex
                })
            },
            {
                "append", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "append",
                    .arity = 1,
                    .callable = appendElementToArrayEnd
                })
            },
            {
                "insertAt", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "insertAt",
                    .arity = 2,
                    .callable = appendElementByIndex
                })
            },

            {
                "pop", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "pop",
                    .arity = 0,
                    .callable = removeElementFromArrayEnd
                })
            },
            {
                "removeAt", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "removeAt",
                    .arity = 1,
                    .callable = removeElementByIndex
                })
            },
            {
                "slice", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "slice",
                    .arity = 2,
                    .callable = sliceArray
                })
            },
            {
                "concat", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "concat",
                    .arity = 1,
                    .callable = concatenateTwoArrays
                })
            },
            {
                "indexOf", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "indexOf",
                    .arity = 1,
                    .callable = retrieveFirstIndexOfArrayElement
                })
            },
            {
                "lastIndexOf", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "lastIndexOf",
                    .arity = 1,
                    .callable = retrieveLastIndexOfArrayElement
                })
            },
            {
                "contains", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "contains",
                    .arity = 1,
                    .callable = containsValue
                })
            },
        };
    }
}
