#include "stdlib/SetBuiltins/SetBuiltins.h"

namespace stdlib::SetBuiltins {
    namespace {
        Value retrieveSetSize(Value receiver, const std::vector<Value> &args) {
            return static_cast<double>(receiver.get<std::shared_ptr<Set>>()->size());
        }

        Value checkSetEmptiness(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Set>>()->empty();
        }

        Value clearSet(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
            receivedObject->clear();
            return receiver;
        }

        Value insertSetElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
            auto [it, success] = receivedObject->emplace(args[0]);
            return success;
        }

        Value removeSetElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
            if (const auto it = receivedObject->find(args[0]); it != receivedObject->end()) {
                receivedObject->erase(it);
                return true;
            }
            return false;
        }

        Value containsSetElement(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
            return receivedObject->contains(args[0]);
        }

        Value applySetUnion(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

            if (!args[0].is<std::shared_ptr<Set>>()) {
                throw std::runtime_error("cannot apply set union operation to a non-set value");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Set>>();

            auto finalUnionSet = *receivedObject;
            finalUnionSet.insert(otherObject->begin(), otherObject->end());
            return std::make_shared<Set>(finalUnionSet);
        }

        Value applySetIntersection(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

            if (!args[0].is<std::shared_ptr<Set>>()) {
                throw std::runtime_error("cannot apply set union operation to a non-set value");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Set>>();

            const auto &smallerSet = receivedObject->size() < otherObject->size() ? receivedObject : otherObject;
            const auto &largerSet = receivedObject->size() < otherObject->size() ? otherObject : receivedObject;

            Set intersectionSet;
            for (const auto &setElement : *smallerSet) {
                if (largerSet->contains(setElement)) {
                    intersectionSet.insert(setElement);
                }
            }

            return std::make_shared<Set>(intersectionSet);
        }

        Value applySetDifference(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

            if (!args[0].is<std::shared_ptr<Set>>()) {
                throw std::runtime_error("cannot apply set union operation to a non-set value");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Set>>();

            Set differenceSet;
            for (const auto &setElement : *receivedObject) {
                if (!otherObject->contains(setElement)) {
                    differenceSet.insert(setElement);
                }
            }

            return std::make_shared<Set>(differenceSet);
        }

        Value applySymmetricDifference(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

            if (!args[0].is<std::shared_ptr<Set>>()) {
                throw std::runtime_error("cannot apply set union operation to a non-set value");
            }

            const auto otherObject = args[0].get<std::shared_ptr<Set>>();

            Set symmetricDifferenceSet;

            for (const auto &setElement : *receivedObject) {
                if (!otherObject->contains(setElement)) {
                    symmetricDifferenceSet.insert(setElement);
                }
            }

            for (const auto &setElement : *otherObject) {
                if (!receivedObject->contains(setElement)) {
                    symmetricDifferenceSet.insert(setElement);
                }
            }

            return std::make_shared<Set>(symmetricDifferenceSet);
        }

        Value createSetClone(Value receiver, const std::vector<Value> &args) {
            return receiver.get<std::shared_ptr<Set>>();
        }

        Value convertSetToArray(Value receiver, const std::vector<Value> &args) {
            const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

            Array convertedArray;
            convertedArray.reserve(receivedObject->size());
            convertedArray.insert(convertedArray.end(), receivedObject->begin(), receivedObject->end());
            return std::make_shared<Array>(convertedArray);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction>> table;
        table["len"] = std::make_shared<NativeFunction>("len", 0, retrieveSetSize);
        table["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkSetEmptiness);
        table["clear"] = std::make_shared<NativeFunction>("clear", 0, clearSet);
        table["insert"] = std::make_shared<NativeFunction>("insert", 1, insertSetElement);
        table["remove"] = std::make_shared<NativeFunction>("remove", 1, removeSetElement);
        table["contains"] = std::make_shared<NativeFunction>("contains", 1, containsSetElement);
        table["union"] = std::make_shared<NativeFunction>("union", 1, applySetUnion);
        table["intersection"] = std::make_shared<NativeFunction>("intersection", 1, applySetIntersection);
        table["difference"] = std::make_shared<NativeFunction>("difference", 1, applySetDifference);
        table["symmetricDifference"] = std::make_shared<NativeFunction>("symmetricDifference", 1, applySymmetricDifference);
        table["copy"] = std::make_shared<NativeFunction>("copy", 0, createSetClone);
        table["toArray"] = std::make_shared<NativeFunction>("toArray", 0, convertSetToArray);
        return table;
    }
}
