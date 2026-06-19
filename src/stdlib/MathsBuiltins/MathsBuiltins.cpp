#include "stdlib/MathsBuiltins/MathsBuiltins.h"

namespace stdlib::MathsBuiltins {
    namespace {
        double retrieve_number_value(const Value &value) {
            if (!value.is<double>()) {
                throw std::invalid_argument(std::format("expected Number but got {}", value.type()));
            }

            return value.get<double>();
        }

        template<typename T>
        std::shared_ptr<NativeFunction> make_one_arity_function(const std::string &name, T &&function) {
            return std::make_shared<NativeFunction>(NativeFunction{
                .name = name,
                .arity = 1,
                .callable = [func = std::forward<T>(function)](const Value &, const std::vector<Value> &args) -> Value {
                    return func(retrieve_number_value(args.front()));
                }
            });
        }

        template<typename T>
        std::shared_ptr<NativeFunction> make_two_arity_function(const std::string &name, T &&function) {
            return std::make_shared<NativeFunction>(NativeFunction{
                .name = name,
                .arity = 2,
                .callable = [func = std::forward<T>(function)](const Value &, const std::vector<Value> &args) -> Value {
                    return func(retrieve_number_value(args[0]), retrieve_number_value(args[1]));
                }
            });
        }

        template<typename T>
        std::shared_ptr<NativeFunction> make_three_arity_function(const std::string &name, T &&function) {
            return std::make_shared<NativeFunction>(NativeFunction{
                .name = name,
                .arity = 3,
                .callable = [func = std::forward<T>(function)](const Value &, const std::vector<Value> &args) -> Value {
                    return func(retrieve_number_value(args[0]), retrieve_number_value(args[1]),
                             retrieve_number_value(args[2]));
                }
            });
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > registerMethods() {
        std::unordered_map<std::string, std::shared_ptr<NativeFunction> > methods_table;

        auto register_one_arity_function = [&methods_table](std::string name, auto &&func) {
            methods_table.emplace(
                name,
                make_one_arity_function(name, std::forward<decltype(func)>(func))
            );
        };

        auto register_two_arity_function = [&methods_table](std::string name, auto &&func) {
            methods_table.emplace(
                name,
                make_two_arity_function(name, std::forward<decltype(func)>(func))
            );
        };

        auto register_three_arity_function = [&methods_table](std::string name, auto &&func) {
            methods_table.emplace(
                name,
                make_three_arity_function(name, std::forward<decltype(func)>(func))
            );
        };

        register_one_arity_function("abs", [](const double &value) { return std::abs(value); });
        register_one_arity_function("sign", [](const double &value) { return value < 0 ? -1.0 : value > 0 ? 1.0 : 0; });
        register_one_arity_function("isEven", [](const double &value) { return std::fmod(value, 2) == 0; });
        register_one_arity_function("isOdd", [](const double &value) { return std::fmod(value, 2) != 0; });
        register_one_arity_function("sqrt", [](const double &value) { return std::sqrt(value); });
        register_one_arity_function("exp", [](const double &value) { return std::exp(value); });
        register_one_arity_function("log", [](const double &value) { return std::log(value); });
        register_one_arity_function("log10", [](const double &value) { return std::log10(value); });
        register_one_arity_function("log2", [](const double &value) { return std::log2(value); });
        register_one_arity_function("floor", [](const double &value) { return std::floor(value); });
        register_one_arity_function("ceil", [](const double &value) { return std::ceil(value); });
        register_one_arity_function("round", [](const double &value) { return std::round(value); });
        register_one_arity_function("trunc", [](const double &value) { return std::trunc(value); });

        register_one_arity_function("fract", [](const double &value) {
            double integer_part;
            return std::modf(value, &integer_part);
        });

        register_one_arity_function("nearby", [](const double &value) { return std::nearbyint(value); });
        register_one_arity_function("rint", [](const double &value) { return std::rint(value); });
        register_one_arity_function("sin", [](const double &value) { return std::sin(value); });
        register_one_arity_function("cos", [](const double &value) { return std::cos(value); });
        register_one_arity_function("tan", [](const double &value) { return std::tan(value); });
        register_one_arity_function("asin", [](const double &value) { return std::asin(value); });
        register_one_arity_function("acos", [](const double &value) { return std::acos(value); });
        register_one_arity_function("atan", [](const double &value) { return std::atan(value); });
        register_one_arity_function("sec", [](const double &value) { return 1.0 / std::cos(value); });
        register_one_arity_function("csc", [](const double &value) { return 1.0 / std::sin(value); });
        register_one_arity_function("cot", [](const double &value) { return 1.0 / std::tan(value); });
        register_one_arity_function("asec", [](const double &value) { return std::acos(1.0 / value); });
        register_one_arity_function("acsc", [](const double &value) { return std::asin(1.0 / value); });

        register_one_arity_function("acot", [](const double &value) {
            if (value > 0) return std::atan(1.0 / value);
            if (value < 0.0) return std::numbers::pi + std::atan(1.0 / value);

            return std::numbers::pi / 2.0;
        });

        register_one_arity_function("factorial", [](const double &value) { return std::tgamma(value + 1.0); });

        register_two_arity_function("min", [](const double &first_operand, const double &second_operand) {
            return std::min(first_operand, second_operand);
        });

        register_two_arity_function("max", [](const double &first_operand, const double &second_operand) {
            return std::max(first_operand, second_operand);
        });

        register_two_arity_function("logn", [](const double &first_operand, const double &second_operand) {
            return std::log(first_operand) / std::log(second_operand);
        });

        register_two_arity_function("countPermutations", [](const double &first_operand, const double &second_operand) {
            double permutations_counter = 1.0;
            for (auto i = 0.0; i < second_operand; i += 1.0) {
                permutations_counter *= first_operand - i;
            }
            return permutations_counter;
        });

        register_two_arity_function("countCombinations", [](const double first_operand, double second_operand) {
            second_operand = std::min(second_operand, first_operand - second_operand);

            double combinations_counter = 1.0;
            for (auto i = 1.0; i <= second_operand; i += 1.0) {
                combinations_counter *= first_operand - second_operand + i;
                combinations_counter /= i;
            }

            return combinations_counter;
        });

        register_three_arity_function("clamp", [](const double &value, const double &lhs, const double &rhs) {
            if (lhs > rhs) {
                throw std::runtime_error(std::format("impossible to restrict value to the specified range [{}; {}]",
                                                     lhs, rhs));
            }

            return std::clamp(value, lhs, rhs);
        });

        return methods_table;
    }

}
