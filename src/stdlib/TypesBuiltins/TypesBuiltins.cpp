#include "stdlib/TypesBuiltins/TypesBuiltins.h"

namespace stdlib::TypesBuiltins {
    namespace {
        String create_string(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<std::string>();
            }

            if (args.front().is<Array>()) {
                const auto &array = args.front().get<Array>();

                if (!args.back().is<String>()) {
                    throw std::invalid_argument("expected some separator");
                }

                auto joined = *array | std::views::transform([](const Value &value) {
                    return value.str();
                }) | std::views::join_with(*args.back().get<String>());

                return std::make_shared<std::string>(std::ranges::to<std::string>(joined));
            }

            if (args.front().is<Set>()) {
                const auto &array = args.front().get<Set>();

                if (!args.back().is<String>()) {
                    throw std::invalid_argument("expected some separator");
                }

                auto joined = *array | std::views::transform([](const Value &value) {
                    return value.str();
                }) | std::views::join_with(*args.back().get<String>());

                return std::make_shared<std::string>(std::ranges::to<std::string>(joined));
            }

            if (args.front().is<double>()) {
                if (!args.back().is<String>()) {
                    throw std::invalid_argument(std::format("expected char but got {}", args.back().type()));
                }

                std::string resulting_string;
                for (auto i = 0; i < args.front().get<double>(); ++i) {
                    resulting_string += *args.back().get<String>();
                }

                return std::make_shared<std::string>(resulting_string);
            }

            throw std::invalid_argument(std::format("no viable constructor for type {}", args.front().type()));
        }

        Set create_set(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<std::unordered_set<Value, ValueHasher> >();
            }

            if (args.front().is<Array>()) {
                const auto &values = args.front().get<Array>();
                return std::make_shared<std::unordered_set<Value, ValueHasher> >(
                    std::ranges::to<std::unordered_set<Value, ValueHasher> >(std::move(*values)));
            }

            if (args.front().is<String>()) {
                const auto &string = args.front().get<String>();

                std::unordered_set<Value, ValueHasher> resulting_set;
                for (const auto &letter: *string) {
                    resulting_set.insert(std::make_shared<std::string>(std::string(1, letter)));
                }

                return std::make_shared<std::unordered_set<Value, ValueHasher> >(std::move(resulting_set));
            }

            throw std::invalid_argument(std::format("expected string or array but got {}", args.front().type()));
        }

        Dictionary create_dictionary(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<std::unordered_map<Value, Value, ValueHasher> >();
            }

            if (args.front().is<Array>()) {
                const auto &pairs = args.front().get<Array>();

                std::unordered_map<Value, Value, ValueHasher> resulting_dictionary;

                for (const auto &pair: *pairs) {
                    if (!pair.is<Array>()) {
                        throw std::invalid_argument(std::format("expected array but got {}", pair.type()));
                    }

                    const auto &array_pair = pair.get<Array>();

                    if (array_pair->size() != 2) {
                        throw std::runtime_error(std::format("cannot create dictionary pair from array of size {}",
                                                             array_pair->size()));
                    }

                    resulting_dictionary.emplace(array_pair->at(0), array_pair->at(1));
                }

                return std::make_shared<std::unordered_map<Value, Value,
                    ValueHasher> >(std::move(resulting_dictionary));
            }

            throw std::invalid_argument(std::format("expected array but got {}", args.front().type()));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {
                "set", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "set",
                    .arity = -1,
                    .callable = create_set
                })
            },
            {
                "dict", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "dict",
                    .arity = -1,
                    .callable = create_dictionary
                })
            },
            {
                "string", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "string",
                    .arity = -1,
                    .callable = create_string
                })
            }
        };
    }
}
