#include "stdlib/StringBuiltins/StringBuiltins.h"

namespace stdlib::StringBuiltins {
    namespace {
        template<typename T>
        decltype(auto) get_from_value(const Value &value) {
            if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
                return (value);
            } else if constexpr (std::is_integral_v<std::remove_cvref_t<T> > && !std::is_same_v<std::remove_cvref_t<
                                     T>, bool>) {
                if (!value.is<double>()) [[unlikely]] {
                    throw std::invalid_argument("string index must be a number");
                }

                const auto val = value.get<double>();

                if (val < 0) [[unlikely]] {
                    throw std::out_of_range("string indices must be non-negative");
                }

                return static_cast<std::remove_cvref_t<T>>(val);
            } else if constexpr (std::is_floating_point_v<std::remove_cvref_t<T> >) {
                if (!value.is<double>()) [[unlikely]] {
                    throw std::invalid_argument("expected number");
                }

                return static_cast<std::remove_cvref_t<T>>(value.get<double>());
            } else {
                if (!value.is<std::remove_cvref_t<T> >()) [[unlikely]] {
                    throw std::invalid_argument(std::format("type mismatch; got {}", value.type()));
                }

                return value.get<std::remove_cvref_t<T> >();
            }
        }

        template<typename T>
        Value convert_to_value(T &&value) {
            if constexpr (std::is_integral_v<std::remove_cvref_t<T> > && !std::is_same_v<std::remove_cvref_t<T>,
                              bool>) {
                return Value(static_cast<double>(value));
            } else if constexpr (std::is_void_v<std::remove_cvref_t<T> >) {
                return NIL{};
            } else {
                return Value(std::forward<T>(value));
            }
        }

        template<auto Func>
        struct MethodTraits;

        template<typename R, typename... Args, R(*Func)(const String &, Args...)>
        struct MethodTraits<Func> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(Value receiver, const std::vector<Value> &args) {
                const auto &string = receiver.get<String>();

                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(string, args, std::make_index_sequence<Arity>{});
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(const String &string, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    Func(string, get_from_value<Args>(args[Is])...);
                    return NIL{};
                } else {
                    return convert_to_value(Func(string, get_from_value<Args>(args[Is])...));
                }
            }
        };

        template<auto Func>
        std::pair<std::string, std::shared_ptr<NativeFunction> > bind_method(const std::string &name) {
            return {
                name,
                std::make_shared<NativeFunction>(NativeFunction{
                    .name = name,
                    .arity = MethodTraits<Func>::Arity,
                    .callable = MethodTraits<Func>::invoke
                })
            };
        }

        double retrieve_string_length(const String &string) {
            return static_cast<double>(string->size());
        }

        String copy_string(const String &string) {
            return std::make_shared<String::element_type>(*string);
        }

        void clear_string(const String &string) {
            string->clear();
        }

        bool check_string_emptiness(const String &string) {
            return string->empty();
        }

        void reverse_string(const String &string) {
            std::ranges::reverse(*string);
        }

        bool contains_substring(const String &string, const String &substring) {
            return string->contains(*substring);
        }

        char retrieve_char_by_index(const String &string, const Value &index) {
            const auto &idx = static_cast<std::size_t>(index.get<double>());

            if (idx >= string->size()) [[unlikely]] {
                throw std::out_of_range(std::format("string index out of bounds"));
            }

            return string->at(idx);
        }

        double retrieve_index_of_substring(const String &string, const String &substring) {
            const auto it = string->find(*substring);
            if (it == std::string::npos) return -1.0;
            return static_cast<double>(it);
        }

        double retrieve_last_index_of_substring(const String &string, const String &substring) {
            const auto it = string->rfind(*substring);
            if (it == std::string::npos) return -1.0;
            return static_cast<double>(it);
        }

        bool starts_with_substring(const String &string, const String &substring) {
            return string->starts_with(*substring);
        }

        bool ends_with_substring(const String &string, const String &substring) {
            return string->ends_with(*substring);
        }

        String take_substring(const String &string, const double &lhs, const double &rhs) {
            const auto &starting_index = static_cast<std::size_t>(lhs);
            const auto &ending_index = static_cast<std::size_t>(rhs);

            if (starting_index >= string->size()) [[unlikely]] {
                return std::make_shared<String::element_type>("");
            }

            return std::make_shared<String::element_type>(string->substr(starting_index, ending_index));
        }

        void convert_to_uppercase(const String &string) {
            std::ranges::transform(*string, string->begin(), [](const unsigned char &symbol) {
                return std::toupper(symbol);
            });
        }

        void convert_to_lowercase(const String &string) {
            std::ranges::transform(*string, string->begin(), [](const unsigned char &symbol) {
                return std::tolower(symbol);
            });
        }

        bool check_alpha(const String &string) {
            return !string->empty() && std::ranges::all_of(*string, [](const auto &symbol) {
                return std::isalpha(symbol);
            });
        }

        bool check_numeric(const String &string) {
            return !string->empty() && std::ranges::all_of(*string, [](const auto &symbol) {
                return std::isdigit(symbol);
            });
        }

        bool check_alpha_numeric(const String &string) {
            return !string->empty() && std::ranges::all_of(*string, [](const auto &symbol) {
                return std::isalnum(symbol);
            });
        }

        bool check_whitespace(const String &string) {
            return !string->empty() && std::ranges::all_of(*string, [](const auto &symbol) {
                return std::isspace(symbol);
            });
        }

        String trim_string(const String &string) {
            const auto &starting_index = string->find_first_not_of(" \t\n\r\f\v");

            if (starting_index == std::string::npos) return std::make_shared<String::element_type>("");

            const auto &ending_index = string->find_last_not_of(" \t\n\r\f\v");
            return std::make_shared<String::element_type>(string->substr(starting_index, ending_index - starting_index + 1));
        }

        void replace_substring(const String &string, const String &original_substring,
                               const String &replacement_substring) {
            if (original_substring->empty()) return;

            if (const auto it = string->find(*original_substring); it != std::string::npos) {
                string->replace(it, original_substring->length(), *replacement_substring);
            }
        }

        void replace_all_substrings(const String &string, const String &original_substring,
                                    const String &replacement_substring) {
            if (original_substring->empty()) return;

            std::size_t it = 0;
            while ((it = string->find(*original_substring, it)) != std::string::npos) {
                string->replace(it, original_substring->length(), *replacement_substring);
                it += replacement_substring->length();
            }
        }

        double convert_string_to_number(const String &string) {
            return std::stod(*string);
        }

        Array split_string(const String &string, const String &separator) {
            auto split_string_view = *string
                                     | std::views::split(*separator)
                                     | std::views::transform([](auto &&range) {
                                         auto substring = std::ranges::to<String::element_type>(range);
                                         return std::make_shared<String::element_type>(std::move(substring));
                                     });

            auto resulting_split_string = std::ranges::to<Array::element_type>(split_string_view);
            return std::make_shared<Array::element_type>(std::move(resulting_split_string));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            bind_method<retrieve_string_length>("len"),
            bind_method<clear_string>("clear"),
            bind_method<copy_string>("copy"),
            bind_method<reverse_string>("reverse"),
            bind_method<check_string_emptiness>("isEmpty"),
            bind_method<contains_substring>("contains"),
            bind_method<retrieve_char_by_index>("at"),
            bind_method<retrieve_index_of_substring>("indexOf"),
            bind_method<retrieve_last_index_of_substring>("lastIndexOf"),
            bind_method<starts_with_substring>("startsWith"),
            bind_method<ends_with_substring>("endsWith"),
            bind_method<take_substring>("substr"),
            bind_method<convert_to_uppercase>("toUpper"),
            bind_method<convert_to_lowercase>("toLower"),
            bind_method<check_alpha>("isAlpha"),
            bind_method<check_numeric>("isNumeric"),
            bind_method<check_alpha_numeric>("isAlphaNumeric"),
            bind_method<check_whitespace>("isWhitespace"),
            bind_method<trim_string>("trim"),
            bind_method<replace_substring>("replace"),
            bind_method<replace_all_substrings>("replaceAll"),
            bind_method<convert_string_to_number>("toNumber"),
            bind_method<split_string>("split"),
        };
    }
}