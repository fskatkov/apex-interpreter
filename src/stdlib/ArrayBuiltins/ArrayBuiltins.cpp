#include "stdlib/ArrayBuiltins/ArrayBuiltins.h"

namespace stdlib::ArrayBuiltins {
    namespace {
        template<typename T>
        decltype(auto) get_from_value(const Value &value) {
            if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
                return (value);
            } else if constexpr (std::is_integral_v<std::remove_cvref_t<T> > && !std::is_same_v<std::remove_cvref_t<
                                     T>, bool>) {
                if (!value.is<double>()) [[unlikely]] {
                    throw std::invalid_argument("array index must be a number");
                }

                const auto val = value.get<double>();

                if (val < 0) [[unlikely]] {
                    throw std::out_of_range("array indices must be non-negative");
                }

                return static_cast<std::remove_cvref_t<T>>(val);
            } else if constexpr (std::is_floating_point_v<std::remove_cvref_t<T> >) {
                if (!value.is<double>()) [[unlikely]] {
                    throw std::invalid_argument("expected Number");
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

        template<typename R, typename... Args, R(*Func)(const std::shared_ptr<Array> &, Args...)>
        struct MethodTraits<Func> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(Value receiver, const std::vector<Value> &args) {
                const auto &array = receiver.get<std::shared_ptr<Array> >();

                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(array, args, std::make_index_sequence<Arity>{});
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(const std::shared_ptr<Array> &array, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    Func(array, get_from_value<Args>(args[Is])...);
                    return NIL{};
                } else {
                    return convert_to_value(Func(array, get_from_value<Args>(args[Is])...));
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

        std::size_t retrieve_array_size(const std::shared_ptr<Array> &array) {
            return array->size();
        }

        std::shared_ptr<Array> clear_array(const std::shared_ptr<Array> &array) {
            array->clear();
            return array;
        }

        std::shared_ptr<Array> copy_array(const std::shared_ptr<Array> &array) {
            return std::make_shared<Array>(*array);
        }

        void reverse_array(const std::shared_ptr<Array> &array) {
            std::ranges::reverse(*array);
        }

        bool check_array_emptiness(const std::shared_ptr<Array> &array) {
            return array->empty();
        }

        Value retrieve_first_array_element(const std::shared_ptr<Array> &array) {
            if (array->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the first element");
            }

            return array->front();
        }

        Value retrieve_last_array_element(const std::shared_ptr<Array> &array) {
            if (array->empty()) [[unlikely]] {
                throw std::runtime_error("array is empty but asked to get the last element");
            }

            return array->back();
        }

        Value retrieve_element_by_index(const std::shared_ptr<Array> &array, const std::size_t &index) {
            if (index >= array->size()) [[unlikely]] {
                throw std::out_of_range("array index out of range");
            }

            return (*array)[index];
        }

        std::shared_ptr<Array> set_element_by_index(const std::shared_ptr<Array> &array, const std::size_t &index,
                                                    const Value &value) {
            if (index >= array->size()) [[unlikely]] {
                throw std::out_of_range("array index out of range");
            }

            (*array)[index] = value;
            return array;
        }

        void add_element_to_array(const std::shared_ptr<Array> &array, const Value &value) {
            array->push_back(value);
        }

        void insert_element_at_index(const std::shared_ptr<Array> &array, const std::size_t &index,
                                     const Value &value) {
            if (index > array->size()) [[unlikely]] {
                throw std::out_of_range("array index out of range");
            }

            array->insert(array->begin() + index, value);
        }

        Value remove_element_from_array(const std::shared_ptr<Array> &array) {
            if (array->empty()) [[unlikely]] {
                throw std::runtime_error("cannot remove from an empty array");
            }

            auto last_element = std::move(array->back());
            array->pop_back();
            return last_element;
        }

        void remove_element_at_index(const std::shared_ptr<Array> &array, const std::size_t &index) {
            if (index >= array->size()) [[unlikely]] {
                throw std::out_of_range("array index out of range");
            }

            array->erase(array->begin() + index);
        }

        std::shared_ptr<Array> slice_array(const std::shared_ptr<Array> &array, const std::size_t &starting_index,
                                           const std::size_t &ending_index) {
            if (starting_index >= array->size() || starting_index > ending_index) [[unlikely]] {
                return std::make_shared<Array>();
            }

            const auto real_ending = std::min(array->size() - 1, ending_index);
            return std::make_shared<Array>(array->begin() + starting_index, array->begin() + real_ending + 1);
        }

        void concat_two_arrays(const std::shared_ptr<Array> &first_operand,
                               const std::shared_ptr<Array> &second_operand) {
            first_operand->reserve(first_operand->size() + second_operand->size());
            first_operand->insert(first_operand->end(), second_operand->begin(), second_operand->end());
        }

        double retrieve_first_index_of(const std::shared_ptr<Array> &array, const Value &value) {
            const auto it = std::ranges::find(*array, value);

            if (it == array->end()) [[unlikely]] {
                throw std::runtime_error(std::format("not found {}", value.str()));
            }

            return static_cast<double>(std::ranges::distance(array->begin(), it));
        }

        double retrieve_last_index_of(const std::shared_ptr<Array> &array, const Value &value) {
            const auto it = std::ranges::find(std::ranges::rbegin(*array), std::ranges::rend(*array), value);

            if (it == array->rend()) [[unlikely]] {
                throw std::runtime_error(std::format("not found {}", value.str()));
            }

            return static_cast<double>(array->size() - 1 - std::distance(std::ranges::rbegin(*array), it));
        }

        bool contains_element(const std::shared_ptr<Array> &array, const Value &value) {
            return std::ranges::contains(*array, value);
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            bind_method<retrieve_array_size>("len"),
            bind_method<clear_array>("clear"),
            bind_method<copy_array>("copy"),
            bind_method<reverse_array>("reverse"),
            bind_method<check_array_emptiness>("isEmpty"),
            bind_method<retrieve_first_array_element>("first"),
            bind_method<retrieve_last_array_element>("last"),
            bind_method<retrieve_element_by_index>("at"),
            bind_method<set_element_by_index>("set"),
            bind_method<add_element_to_array>("append"),
            bind_method<insert_element_at_index>("insertAt"),
            bind_method<remove_element_from_array>("pop"),
            bind_method<remove_element_at_index>("removeAt"),
            bind_method<slice_array>("slice"),
            bind_method<concat_two_arrays>("concat"),
            bind_method<retrieve_first_index_of>("indexOf"),
            bind_method<retrieve_last_index_of>("lastIndexOf"),
            bind_method<contains_element>("contains"),
        };
    }
}