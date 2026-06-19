#include "stdlib/MathsBuiltins/MathsBuiltins.h"

namespace stdlib::MathsBuiltins {
    namespace {
        Value take_absolute_value(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::abs(args.front().get<double>());
        }

        Value check_value_sign(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            const auto &value = args.front().get<double>();
            return value < 0 ? -1.0 : value > 0 ? 1.0 : 0.0;
        }

        Value find_min_value(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>() || !args[1].is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {} and {}", args.front().type(), args[1].type()));
            }

            return std::min(args.front().get<double>(), args[1].get<double>());
        }

        Value find_max_value(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>() || !args[1].is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {} and {}", args.front().type(), args[1].type()));
            }

            return std::max(args.front().get<double>(), args[1].get<double>());
        }

        Value restrict_value_to_range(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>() || !args[1].is<double>() || !args[2].is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}, {} and {}", args.front().type(), args[1].type(), args[2].type()));
            }

            if (args[1].is<double>() > args[2].is<double>()) {
                throw std::runtime_error(std::format("impossible to restrive value to the specified range [{}; {}]", args[1].get<double>(), args[2].get<double>()));
            }

            return std::clamp(args.front().get<double>(), args[1].get<double>(), args[2].get<double>());
        }

    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        return {
            {
                "abs", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "abs",
                    .arity = 1,
                    .callable = take_absolute_value
                })
            },
            {
                "sign", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "sign",
                    .arity = 1,
                    .callable = check_value_sign
                })
            },
            {
                "min", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "min",
                    .arity = 2,
                    .callable = find_min_value
                })
            },
            {
                "max", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "max",
                    .arity = 2,
                    .callable = find_max_value
                })
            },
            {
                "clamp", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "clamp",
                    .arity = 3,
                    .callable = restrict_value_to_range
                })
            },
        };
    }
}
