#include "stdlib/TypesBuiltins/TypesBuiltins.h"

namespace stdlib::TypesBuiltins {
    namespace {
        Array create_array(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<Array::element_type>();
            }

            return std::visit(overloaded{
                [&](const double &array_size) -> Array  {
                    return std::make_shared<Array::element_type>(static_cast<std::size_t>(array_size), args.back());
                },
                [&](const String &string_container) -> Array {
                    if (args.size() < 2 || !args.back().is<String>()) {
                        throw std::invalid_argument("expected some separator");
                    }

                    auto view = *string_container
                                | std::views::split(*args.back().get<String>())
                                | std::views::transform([](auto &&range) -> Value {
                                    return std::make_shared<String::element_type>(
                                        std::ranges::to<String::element_type>(range));
                                });

                    return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(view));
                },
                [](const Set &set_container) -> Array {
                    return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(*set_container));
                },
                [](const Dictionary &dictionary_container) -> Array {
                    auto view = *dictionary_container | std::views::transform([](const auto &pair) -> Value {
                        return std::make_shared<Array::element_type>(Array::element_type{pair.first, pair.second});
                    });

                    return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(view));
                },
                [&](const auto &) -> Array {
                    throw std::invalid_argument(std::format("no viable array constructor for {}", args.front().type()));
                },
            }, args.front().as);
        }

        String create_string(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<String::element_type>();
            }

            if (args.size() == 1) {
                return std::make_shared<String::element_type>(args.front().str());
            }

            auto join_container = [&](const auto &container_to_join) -> String {
                if (!args.back().is<String>()) {
                    throw std::invalid_argument("expected string separator");
                }

                auto joined = *container_to_join
                            | std::views::transform([](const Value &value) { return value.str(); })
                            | std::views::join_with(*args.back().get<String>());

                return std::make_shared<String::element_type>(std::ranges::to<String::element_type>(joined));
            };

            return std::visit(overloaded{
                [&](const Array &array_container) -> String {
                    return join_container(array_container);
                },
                [&](const Set &set_container) -> String {
                    return join_container(set_container);
                },
                [&](const double &size_of_string) -> String {
                    if (!args.back().is<String>()) {
                        throw std::invalid_argument(std::format("expected string but got {}", args.back().type()));
                    }

                    const auto &repeated_element = *args.back().get<String>();

                    std::string resulting_string;
                    resulting_string.reserve(repeated_element.size() * static_cast<std::size_t>(size_of_string));

                    for (auto i = 0; i < size_of_string; ++i) {
                        resulting_string += repeated_element;
                    }

                    return std::make_shared<String::element_type>(std::move(resulting_string));
                },
                [&](const auto &) -> String {
                    throw std::invalid_argument(std::format("no viable string constructor for {}", args.front().type()));
                }
            }, args.front().as);
        }

        Set create_set(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<Set::element_type>();
            }

            return std::visit(overloaded{
                [](const Array &array) -> Set {
                    return std::make_shared<Set::element_type>(std::ranges::to<Set::element_type>(*array));
                },
                [](const String &string) -> Set {
                    auto view = *string | std::views::transform([](const char &symbol) -> Value {
                        return std::make_shared<std::string>(1, symbol);
                    });

                    return std::make_shared<Set::element_type>(std::ranges::to<Set::element_type>(view));
                },
                [&](const auto&) -> Set {
                    throw std::invalid_argument(std::format("no viable set constructor for {}", args.front().type()));
                }
            }, args.front().as);
        }

        Dictionary create_dictionary(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                return std::make_shared<Dictionary::element_type>();
            }

            return std::visit(overloaded{
                [](const Array &array) -> Dictionary {
                    auto view = *array | std::views::transform([](const Value &pair) {
                        if (!pair.is<Array>()) {
                            throw std::invalid_argument(std::format("expected array but got {}", pair.type()));
                        }

                        const auto &array_pair = pair.get<Array>();
                        if (array_pair->size() != 2) {
                            throw std::invalid_argument(std::format("cannot use `dict` with an array of size {}", array_pair->size()));
                        }

                        return std::pair{(*array_pair)[0], (*array_pair)[1]};
                    });

                    return std::make_shared<Dictionary::element_type>(std::ranges::to<Dictionary::element_type>(view));
                },
                [&](const auto&) -> Dictionary {
                    throw std::invalid_argument(std::format("no viable dictionary constructor for {}", args.front().type()));
                }
            }, args.front().as);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {
                "array", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "array",
                    .arity = -1,
                    .callable = create_array
                })
            },
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
