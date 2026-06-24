#include "stdlib/DictionaryBuiltins/DictionaryBuiltins.h"

namespace stdlib::DictionaryBuiltins {
    namespace {
        template<typename T>
        decltype(auto) get_from_value(const Value &value) {
            if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
                return (value);
            } else if constexpr (std::is_integral_v<std::remove_cvref_t<T> > && !std::is_same_v<std::remove_cvref_t<
                                     T>, bool>) {
                if (!value.is<double>() || !value.is<std::string>()) [[unlikely]] {
                    throw std::invalid_argument("dictionary index must be a number or string");
                }

                return static_cast<std::remove_cvref_t<T>>(value.get<double>());
            } else if constexpr (std::is_floating_point_v<std::remove_cvref_t<T> >) {
                if (!value.is<double>()) [[unlikely]] {
                    throw std::invalid_argument("expected number");
                }

                return static_cast<std::remove_cvref_t<T>>(value.get<double>());
            } else {
                if (!value.is<std::remove_cvref_t<T> >()) [[unlikely]] {
                    throw std::invalid_argument(std::format("type mismatch; got {}", value.type()));
                }

                return (value.get<std::remove_cvref_t<T> >());
            }
        }

        template<typename T>
        Value convert_to_value(T &&value) {
            if constexpr (std::is_integral_v<std::remove_cvref_t<T> > && !std::is_same_v<std::remove_cvref_t<T>, bool>) {
                return Value(static_cast<double>(value));
            } else if constexpr (std::is_void_v<std::remove_cvref_t<T> >) {
                return NIL{};
            } else {
                return Value(std::forward<T>(value));
            }
        }

        template<auto Func>
        struct MethodTraits;

        template<typename R, typename... Args, R(*Func)(const Dictionary &, Args...)>
        struct MethodTraits<Func> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(Value receiver, const std::vector<Value> &args) {
                const auto &dictionary = receiver.get<Dictionary>();

                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(dictionary, args, std::make_index_sequence<Arity>{});
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(const Dictionary &dictionary, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    return NIL{};
                } else {
                    return convert_to_value(Func(dictionary, get_from_value<Args>(args[Is])...));
                }
            }
        };

        template<auto Func>
        std::pair<std::string, std::shared_ptr<NativeFunction> > bind_methods(const std::string &name) {
            return {
                name,
                std::make_shared<NativeFunction>(NativeFunction{
                    .name = name,
                    .arity = MethodTraits<Func>::Arity,
                    .callable = MethodTraits<Func>::invoke
                })
            };
        }

        double retrieve_dictionary_length(const Dictionary &dictionary) {
            return static_cast<double>(dictionary->size());
        }

        Dictionary copy_dictionary(const Dictionary &dictionary) {
            return std::make_shared<Dictionary::element_type>(*dictionary);
        }

        Dictionary clear_dictionary(const Dictionary &dictionary) {
            dictionary->clear();
            return dictionary;
        }

        bool check_dictionary_emptiness(const Dictionary &dictionary) {
            return dictionary->empty();
        }

        bool contains_dictionary_key(const Dictionary &dictionary, const Value &key) {
            return dictionary->contains(key);
        }

        Value get_dictionary_element_by_key(const Dictionary &dictionary, const Value &key, const Value &default_value) {
            if (const auto it = dictionary->find(key); it != dictionary->end()) {
                return it->second;
            }

            return default_value;
        }

        bool set_dictionary_element_by_key(const Dictionary &dictionary, const Value &key, const Value &value) {
            auto [it, inserted_element] = dictionary->try_emplace(key, value);
            return inserted_element;
        }

        Array retrieve_dictionary_keys(const Dictionary &dictionary) {
            return std::make_shared<Array::element_type>(
                std::views::keys(*dictionary) | std::ranges::to<Array::element_type>());
        }

        Array retrieve_dictionary_values(const Dictionary &dictionary) {
            return std::make_shared<Array::element_type>(
                std::views::values(*dictionary) | std::ranges::to<Array::element_type>());
        }

        Dictionary merge_dictionaries(const Dictionary &first_operand, const Dictionary &second_operand) {
            for (const auto &[key, value]: *second_operand) {
                first_operand->insert_or_assign(key, value);
            }

            return first_operand;
        }

        Value remove_dictionary_element_by_key(const Dictionary &dictionary, const Value &key) {
            if (const auto it = dictionary->find(key); it != dictionary->end()) {
                auto extracted_value = std::move(it->second);
                dictionary->erase(it);
                return extracted_value;
            }

            throw std::runtime_error("dictionary key does not exist");
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            bind_methods<retrieve_dictionary_length>("len"),
            bind_methods<clear_dictionary>("clear"),
            bind_methods<copy_dictionary>("copy"),
            bind_methods<check_dictionary_emptiness>("isEmpty"),
            bind_methods<contains_dictionary_key>("contains"),
            bind_methods<get_dictionary_element_by_key>("get"),
            bind_methods<set_dictionary_element_by_key>("set"),
            bind_methods<retrieve_dictionary_keys>("keys"),
            bind_methods<retrieve_dictionary_values>("values"),
            bind_methods<merge_dictionaries>("merge"),
            bind_methods<remove_dictionary_element_by_key>("remove"),
        };
    }
}
