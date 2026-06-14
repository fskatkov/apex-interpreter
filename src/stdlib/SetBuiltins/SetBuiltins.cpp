#include "stdlib/SetBuiltins/SetBuiltins.h"

namespace stdlib::SetBuiltins {
    static Value retrieveSetSize(Value receiver, const std::vector<Value> &args) {
        return static_cast<double>(receiver.get<std::shared_ptr<Set>>()->size());
    }

    static Value checkSetEmptiness(Value receiver, const std::vector<Value> &args) {
        return receiver.get<std::shared_ptr<Set>>()->empty();
    }

    static Value clearSet(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
        receivedObject->clear();
        return receiver;
    }

    static Value insertSetElement(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
        auto [it, success] = receivedObject->emplace(args[0]);
        return success;
    }

    static Value removeSetElement(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
        if (const auto it = receivedObject->find(args[0]); it != receivedObject->end()) {
            receivedObject->erase(it);
            return true;
        }
        return false;
    }

    static Value containsSetElement(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();
        return receivedObject->contains(args[0]);
    }

    static Value applySetUnion(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

        if (!args[0].is<std::shared_ptr<Set>>()) {
            throw std::runtime_error("cannot apply set union operation to a non-set value");
        }

        const auto otherObject = args[0].get<std::shared_ptr<Set>>();

        auto finalUnionSet = *receivedObject;
        finalUnionSet.insert(otherObject->begin(), otherObject->end());
        return std::make_shared<Set>(finalUnionSet);
    }

    static Value applySetIntersection(Value receiver, const std::vector<Value> &args) {
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

    static Value applySetDifference(Value receiver, const std::vector<Value> &args) {
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

    static Value applySymmetricDifference(Value receiver, const std::vector<Value> &args) {
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

    static Value createSetClone(Value receiver, const std::vector<Value> &args) {
        return receiver.get<std::shared_ptr<Set>>();
    }

    static Value convertSetToArray(Value receiver, const std::vector<Value> &args) {
        const auto receivedObject = receiver.get<std::shared_ptr<Set>>();

        Array convertedArray;
        convertedArray.reserve(receivedObject->size());
        convertedArray.insert(convertedArray.end(), receivedObject->begin(), receivedObject->end());
        return std::make_shared<Array>(convertedArray);
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["size"] = std::make_shared<NativeFunction>("size", 0, retrieveSetSize);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkSetEmptiness);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearSet);

        registry["insert"] = std::make_shared<NativeFunction>("insert", 1, insertSetElement);
        registry["remove"] = std::make_shared<NativeFunction>("remove", 1, removeSetElement);
        registry["contains"] = std::make_shared<NativeFunction>("contains", 1, containsSetElement);

        registry["union"] = std::make_shared<NativeFunction>("union", 1, applySetUnion);
        registry["intersection"] = std::make_shared<NativeFunction>("intersection", 1, applySetIntersection);
        registry["difference"] = std::make_shared<NativeFunction>("difference", 1, applySetDifference);
        registry["symmetricDifference"] = std::make_shared<NativeFunction>("symmetricDifference", 1, applySymmetricDifference);

        registry["copy"] = std::make_shared<NativeFunction>("copy", 0, createSetClone);
        registry["toArray"] = std::make_shared<NativeFunction>("toArray", 0, convertSetToArray);
    }
}
