#include "stdlib/ArrayLib/ArrayLib.h"

namespace stdlib::ArrayLib {
    static Value push(Value receiver, const std::vector<Value> &args) {
        if (!receiver.is<std::shared_ptr<Array>>()) {
            return NIL{};
        }

        const auto receivedObject = receiver.get<std::shared_ptr<Array>>();
        receivedObject->push_back(args[0]);
        return NIL{};
    }

    void registerMethods(std::unordered_map<std::string, std::shared_ptr<NativeFunction>>& registry) {
        registry["push"] = std::make_shared<NativeFunction>("push", 1, push);
    }
}
