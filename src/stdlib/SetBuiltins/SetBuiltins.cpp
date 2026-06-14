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

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["size"] = std::make_shared<NativeFunction>("size", 0, retrieveSetSize);
        registry["isEmpty"] = std::make_shared<NativeFunction>("isEmpty", 0, checkSetEmptiness);
        registry["clear"] = std::make_shared<NativeFunction>("clear", 0, clearSet);
    }
}
