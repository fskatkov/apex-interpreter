#include "stdlib/SetBuiltins/SetBuiltins.h"

namespace stdlib::SetBuiltins {
    namespace {
        template<typename T>
        decltype(auto) get_from_value(const Value &value) {
            if constexpr (std::is_same_v<std::remove_cvref_t<T>, Value>) {
                return (value);
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

        template<typename R, typename... Args, R(*Func)(const Set &, Args...)>
        struct MethodTraits<Func> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(Value receiver, const std::vector<Value> &args) {
                const auto &set = receiver.get<Set>();

                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(set, args, std::make_index_sequence<Arity>());
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(const Set &set, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    Func(set, get_from_value<Args>(args[Is])...);
                    return NIL{};
                } else {
                    return convert_to_value(Func(set, get_from_value<Args>(args[Is])...));
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

        double retrieve_set_size(const Set &set) {
            return static_cast<double>(set->size());
        }

        bool check_set_emptiness(const Set &set) {
            return set->empty();
        }

        Set clear_set(const Set &set) {
            set->clear();
            return set;
        }

        Set copy_set(const Set &set) {
            return std::make_shared<Set::element_type>(*set);
        }

        bool insert_element_to_set(const Set &set, const Value &value) {
            return set->emplace(value).second;
        }

        bool remove_element_from_set(const Set &set, const Value &value) {
            return set->erase(value) > 0;
        }

        bool contains_set_element(const Set &set, const Value &value) {
            return set->contains(value);
        }

        Array convert_set_to_array(const Set &set) {
            return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(*set));
        }


        Set apply_set_union(const Set &first_operand, const Set &second_operand) {
            auto final_union_set = *first_operand;
            final_union_set.reserve(final_union_set.size() + second_operand->size());
            final_union_set.insert(second_operand->begin(), second_operand->end());
            return std::make_shared<Set::element_type>(std::move(final_union_set));
        }

        Set apply_set_intersection(const Set &first_operand, const Set &second_operand) {
            const auto &smaller_set = first_operand->size() < second_operand->size()
                                          ? first_operand
                                          : second_operand;
            const auto &larger_set = first_operand->size() < second_operand->size()
                                         ? second_operand
                                         : first_operand;

            Set::element_type final_intersection_set;
            final_intersection_set.reserve(smaller_set->size());

            for (const auto &set_element: *smaller_set) {
                if (larger_set->contains(set_element)) {
                    final_intersection_set.emplace(set_element);
                }
            }

            return std::make_shared<Set::element_type>(std::move(final_intersection_set));
        }

        Set apply_set_difference(const Set &first_operand, const Set &second_operand) {
            Set::element_type final_difference_set;
            final_difference_set.reserve(first_operand->size());

            for (const auto &set_element: *first_operand) {
                if (!second_operand->contains(set_element)) {
                    final_difference_set.emplace(set_element);
                }
            }

            return std::make_shared<Set::element_type>(std::move(final_difference_set));
        }

        Set apply_set_symmetric_difference(const Set &first_operand, const Set &second_operand) {
            Set::element_type final_symmetric_difference_set;
            final_symmetric_difference_set.reserve(first_operand->size() + second_operand->size());

            for (const auto &set_element: *first_operand) {
                if (!second_operand->contains(set_element)) {
                    final_symmetric_difference_set.emplace(set_element);
                }
            }

            for (const auto &set_element: *second_operand) {
                if (!first_operand->contains(set_element)) {
                    final_symmetric_difference_set.emplace(set_element);
                }
            }

            return std::make_shared<Set::element_type>(std::move(final_symmetric_difference_set));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            bind_method<retrieve_set_size>("len"),
            bind_method<clear_set>("clear"),
            bind_method<copy_set>("copy"),
            bind_method<convert_set_to_array>("toArray"),
            bind_method<check_set_emptiness>("isEmpty"),
            bind_method<insert_element_to_set>("insert"),
            bind_method<remove_element_from_set>("remove"),
            bind_method<contains_set_element>("contains"),
            bind_method<apply_set_union>("union"),
            bind_method<apply_set_intersection>("intersection"),
            bind_method<apply_set_difference>("difference"),
            bind_method<apply_set_symmetric_difference>("symmetricDifference"),
        };
    }
}
