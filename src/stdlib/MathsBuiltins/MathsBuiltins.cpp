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

        Value take_square_root(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::sqrt(args.front().get<double>());
        }

        Value compute_exponent(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::exp(args.front().get<double>());
        }

        Value compute_log(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::log(args.front().get<double>());
        }

        Value compute_log10(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::log10(args.front().get<double>());
        }

        Value compute_log2(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::log2(args.front().get<double>());
        }

        Value compute_log_n(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>() || !args[1].is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {} and {}", args.front().type(), args[1].type()));
            }

            return std::log(args.front().get<double>()) / std::log(args[1].get<double>());
        }

        Value compute_floor(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::floor(args.front().get<double>());
        }

        Value compute_ceil(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::ceil(args.front().get<double>());
        }

        Value round_number(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::round(args.front().get<double>());
        }

        Value truncate_number(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::trunc(args.front().get<double>());
        }

        Value take_fraction(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            double integer_part;
            return std::modf(args.front().get<double>(), &integer_part);
        }

        Value find_nearby(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::nearbyint(args.front().get<double>());
        }

        Value find_rint(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::rint(args.front().get<double>());
        }

        Value compute_sin(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::sin(args.front().get<double>());
        }

        Value compute_cos(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::cos(args.front().get<double>());
        }

        Value compute_tan(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::tan(args.front().get<double>());
        }

        Value compute_asin(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::asin(args.front().get<double>());
        }

        Value compute_acos(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::acos(args.front().get<double>());
        }

        Value compute_atan(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return std::atan(args.front().get<double>());
        }

        Value compute_sec(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return 1.0 / std::cos(args.front().get<double>());
        }

        Value compute_csc(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return 1.0 / std::sin(args.front().get<double>());
        }

        Value compute_cot(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            return 1.0 / std::tan(args.front().get<double>());
        }

        Value compute_asec(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            if (args.front().get<double>() < 1) {
                throw std::invalid_argument("cannot work with number less than one");
            }

            return std::acos(1.0 / args.front().get<double>());
        }

        Value compute_acsc(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            if (args.front().get<double>() < 1) {
                throw std::invalid_argument("cannot work with number less than one");
            }

            return std::asin(1.0 / args.front().get<double>());
        }

        Value compute_acot(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", args.front().type()));
            }

            constexpr double pi = std::numbers::pi;
            const auto &value = args.front().get<double>();

            if (value > 0) return std::atan(1.0 / value);
            if (value < 0) return pi + std::atan(1.0 / value);

            return pi / 2.0;
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
            {
                "sqrt", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "sqrt",
                    .arity = 1,
                    .callable = take_square_root
                })
            },
            {
                "exp", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "exp",
                    .arity = 1,
                    .callable = compute_exponent
                })
            },
            {
                "log", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "log",
                    .arity = 1,
                    .callable = compute_log
                })
            },
            {
                "log10", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "log10",
                    .arity = 1,
                    .callable = compute_log10
                })
            },
            {
                "log2", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "log2",
                    .arity = 1,
                    .callable = compute_log2
                })
            },
            {
                "logn", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "logn",
                    .arity = 2,
                    .callable = compute_log_n
                })
            },
            {
                "floor", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "floor",
                    .arity = 1,
                    .callable = compute_floor
                })
            },
            {
                "ceil", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "ceil",
                    .arity = 1,
                    .callable = compute_ceil
                })
            },
            {
                "round", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "round",
                    .arity = 1,
                    .callable = round_number
                })
            },
            {
                "trunc", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "trunc",
                    .arity = 1,
                    .callable = truncate_number
                })
            },
            {
                "fract", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "fract",
                    .arity = 1,
                    .callable = take_fraction
                })
            },
            {
                "nearby", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "nearby",
                    .arity = 1,
                    .callable = find_nearby
                })
            },
            {
                "rint", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "rint",
                    .arity = 1,
                    .callable = find_rint
                })
            },
            {
                "sin", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "sin",
                    .arity = 1,
                    .callable = compute_sin
                })
            },
            {
                "cos", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "cos",
                    .arity = 1,
                    .callable = compute_cos
                })
            },
            {
                "tan", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "tan",
                    .arity = 1,
                    .callable = compute_tan
                })
            },
            {
                "asin", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "asin",
                    .arity = 1,
                    .callable = compute_asin
                })
            },
            {
                "acos", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "acos",
                    .arity = 1,
                    .callable = compute_acos
                })
            },
            {
                "atan", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "atan",
                    .arity = 1,
                    .callable = compute_atan
                })
            },
            {
                "sec", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "sec",
                    .arity = 1,
                    .callable = compute_sec
                })
            },
            {
                "csc", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "csc",
                    .arity = 1,
                    .callable = compute_csc
                })
            },
            {
                "cot", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "cot",
                    .arity = 1,
                    .callable = compute_cot
                })
            },
            {
                "asec", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "asec",
                    .arity = 1,
                    .callable = compute_asec
                })
            },
            {
                "acsc", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "acsc",
                    .arity = 1,
                    .callable = compute_acsc
                })
            },
            {
                "acot", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "acot",
                    .arity = 1,
                    .callable = compute_acot
                })
            },
        };
    }
}
