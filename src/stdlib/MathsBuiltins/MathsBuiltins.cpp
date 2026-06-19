#include "stdlib/MathsBuiltins/MathsBuiltins.h"

namespace stdlib::MathsBuiltins {
    namespace {
        Value take_absolute_value(const Value&, const std::vector<Value>& args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::abs(args.front().get<double>());
        }

    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction>> registerMethods() {
        return {
            {
                "abs", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "abs",
                    .arity = 1,
                    .callable = take_absolute_value
                })
            }
        };
    }

}
