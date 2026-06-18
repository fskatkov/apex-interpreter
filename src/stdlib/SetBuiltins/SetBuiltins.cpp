#include "stdlib/SetBuiltins/SetBuiltins.h"

namespace stdlib::SetBuiltins {
    namespace {
        Value retrieveSetSize(Value receiver, const std::vector<Value> &) {
            return static_cast<double>(receiver.get<std::shared_ptr<Set> >()->size());
        }

        Value checkSetEmptiness(Value receiver, const std::vector<Value> &) {
            return receiver.get<std::shared_ptr<Set> >()->empty();
        }

        Value clearSet(Value receiver, const std::vector<Value> &) {
            receiver.get<std::shared_ptr<Set> >()->clear();
            return receiver;
        }

        Value insertSetElement(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Set> >()->emplace(args.front()).second;
        }

        Value removeSetElement(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Set> >()->erase(args.front()) > 0;
        }

        Value containsSetElement(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Set> >()->contains(args.front());
        }

        Value applySetUnion(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Set> >()) [[unlikely]] {
                throw std::runtime_error("cannot apply set union to a non-set value");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Set> >();
            const auto &otherObject = args.front().get<std::shared_ptr<Set> >();

            auto finalUnionSet = *receivedObject;
            finalUnionSet.reserve(finalUnionSet.size() + otherObject->size());
            finalUnionSet.insert(otherObject->begin(), otherObject->end());
            return std::make_shared<Set>(std::move(finalUnionSet));
        }

        Value applySetIntersection(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Set> >()) [[unlikely]] {
                throw std::runtime_error("cannot apply set intersection to a non-set value");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Set> >();
            const auto &otherObject = args.front().get<std::shared_ptr<Set> >();

            const auto &smallerSet = receivedObject->size() < otherObject->size()
                                         ? receivedObject
                                         : otherObject;
            const auto &largerSet = receivedObject->size() < otherObject->size()
                                        ? otherObject
                                        : receivedObject;

            Set intersectionSet;
            intersectionSet.reserve(smallerSet->size());

            for (const auto &setElement: *smallerSet) {
                if (largerSet->contains(setElement)) {
                    intersectionSet.emplace(setElement);
                }
            }

            return std::make_shared<Set>(std::move(intersectionSet));
        }

        Value applySetDifference(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Set> >()) [[unlikely]] {
                throw std::runtime_error("cannot apply set difference to a non-set value");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Set> >();
            const auto &otherObject = args.front().get<std::shared_ptr<Set> >();

            Set differenceSet;
            differenceSet.reserve(receivedObject->size());
            for (const auto &setElement: *receivedObject) {
                if (!otherObject->contains(setElement)) {
                    differenceSet.emplace(setElement);
                }
            }

            return std::make_shared<Set>(std::move(differenceSet));
        }

        Value applySymmetricDifference(Value receiver, const std::vector<Value> &args) {
            if (!args.front().is<std::shared_ptr<Set> >()) [[unlikely]] {
                throw std::runtime_error("cannot apply set symmetric difference to a non-set value");
            }

            const auto &receivedObject = receiver.get<std::shared_ptr<Set> >();
            const auto &otherObject = args.front().get<std::shared_ptr<Set> >();

            Set symmetricDifferenceSet;
            symmetricDifferenceSet.reserve(receivedObject->size() + otherObject->size());

            for (const auto &setElement: *receivedObject) {
                if (!otherObject->contains(setElement)) {
                    symmetricDifferenceSet.emplace(setElement);
                }
            }

            for (const auto &setElement: *otherObject) {
                if (!receivedObject->contains(setElement)) {
                    symmetricDifferenceSet.emplace(setElement);
                }
            }

            return std::make_shared<Set>(std::move(symmetricDifferenceSet));
        }

        Value createSetClone(Value receiver, const std::vector<Value> &) {
            return std::make_shared<Set>(*receiver.get<std::shared_ptr<Set> >());
        }

        Value convertSetToArray(Value receiver, const std::vector<Value> &) {
            const auto &receivedObject = receiver.get<std::shared_ptr<Set> >();
            return std::make_shared<Array>(std::ranges::to<Array>(*receivedObject));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "len", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "len",
                    .arity = 0,
                    .callable = retrieveSetSize
                })
            },
            {
                "clear", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "clear",
                    .arity = 0,
                    .callable = clearSet
                })
            },
            {
                "copy", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "copy",
                    .arity = 0,
                    .callable = createSetClone
                })
            },
            {
                "isEmpty", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "isEmpty",
                    .arity = 0,
                    .callable = checkSetEmptiness
                })
            },
            {
                "insert", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "insert",
                    .arity = 1,
                    .callable = insertSetElement
                })
            },
            {
                "remove", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "remove",
                    .arity = 1,
                    .callable = removeSetElement
                })
            },
            {
                "contains", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "contains",
                    .arity = 1,
                    .callable = containsSetElement
                })
            },
            {
                "union", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "union",
                    .arity = 1,
                    .callable = applySetUnion
                })
            },
            {
                "intersection", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "intersection",
                    .arity = 1,
                    .callable = applySetIntersection
                })
            },
            {
                "difference", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "difference",
                    .arity = 1,
                    .callable = applySetDifference
                })
            },
            {
                "symmetricDifference", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "symmetricDifference",
                    .arity = 1,
                    .callable = applySymmetricDifference
                })
            },

            {
                "toArray", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "toArray",
                    .arity = 0,
                    .callable = convertSetToArray
                })
            }
        };
    }
}
