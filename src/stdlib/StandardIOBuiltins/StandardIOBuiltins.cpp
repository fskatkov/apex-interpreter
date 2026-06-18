#include "stdlib/StandardIOBuiltins/StandardIOBuiltins.h"

namespace stdlib::StandardIOBuiltins {
    namespace {
        Value builtin_println(const Value&, const std::vector<Value> &args) {
            for (auto i = 0; i < args.size(); ++i) {
                std::cout << args[i].str();
            }
            std::cout << std::endl;
            return NIL{};
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "println", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "println",
                    .arity = -1,
                    .callable = builtin_println
                })
            }
        };
    }

}
