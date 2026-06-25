#include "stdlib/UtilitiesBuiltins/UtilitiesBuiltins.h"

namespace stdlib::UtilitiesBuiltins {
    namespace {
        template<typename T>
        decltype(auto) get_from_value(const Value &value) {
            if (!value.is<std::decay_t<T> >()) {
                throw std::invalid_argument(std::format("mismatch: got `{}`", value.type()));
            }

            return value.get<std::decay_t<T> >();
        }

        template<typename T>
        concept standard_binding = requires(T func, const Value &value, const std::vector<Value> &args)
        {
            { func(value, args) } -> std::convertible_to<Value>;
        };

        template<typename Func>
        struct MethodTraits;

        template<typename R, typename... Args>
        struct MethodTraits<R(*)(Args...)> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(auto func, const std::vector<Value> &args) {
                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(func, args, std::index_sequence_for<Args...>{});
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(auto func, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    func(get_from_value<Args>(args[Is])...);
                    return NIL{};
                } else {
                    return func(get_from_value<Args>(args[Is])...);
                }
            }
        };

        template<auto Func>
        std::shared_ptr<NativeFunction> bind_function(std::string name, const int arity = -1) {
            std::function<Value(Value, const std::vector<Value> &)> callable;
            auto resulting_arity = arity;

            if constexpr (standard_binding<decltype(Func)>) {
                callable = [](const Value &receiver, const std::vector<Value> &args) -> Value {
                    return Func(receiver, args);
                };
            } else {
                if (resulting_arity == -1) {
                    resulting_arity = MethodTraits<decltype(Func)>::Arity;
                }

                callable = [](Value, const std::vector<Value> &args) -> Value {
                    return MethodTraits<decltype(Func)>::invoke(Func, args);
                };
            }

            return std::make_shared<NativeFunction>(NativeFunction{
                .name = std::move(name),
                .arity = resulting_arity,
                .callable = std::move(callable)
            });
        }

        NIL builtin_println(const Value &, const std::vector<Value> &args) {
            for (const auto &arg: args) {
                std::cout << arg.str();
            }
            std::cout << std::endl;
            return NIL{};
        }

        String builtin_read_line(const Value &, const std::vector<Value> &args) {
            if (!args.empty() && args.front().is<String>()) {
                std::cout << *args.front().get<String>() << std::flush;
            }

            if (std::string input; std::getline(std::cin, input)) {
                return std::make_shared<String::element_type>(std::move(input));
            }

            throw std::runtime_error("invalid input format");
        }

        File builtin_open_file(const String &original_path) {
            std::filesystem::path path(*original_path);
            return std::make_shared<File::element_type>(path);
        }

        Array builtin_range(const Value &, const std::vector<Value> &args) {
            if (args.size() < 2 || args.size() > 3) {
                throw std::invalid_argument(std::format("expected 2 or 3 arguments, but got {}", args.size()));
            }

            const auto start_side = get_from_value<double>(args[0]);
            const auto end_side = get_from_value<double>(args[1]);
            const auto step = args.size() == 3 ? get_from_value<double>(args[2]) : 1.0;

            if ((step > 0 && start_side > end_side) || (step < 0 && start_side < end_side) || step == 0) {
                throw std::runtime_error(std::format("cannot generate sequence [{}; {})", start_side, end_side));
            }

            auto sequence_length = static_cast<int>(std::floor((end_side - start_side) / step)) + 1;
            auto range = std::views::iota(0, sequence_length)
                         | std::views::transform([=](const auto &i) { return start_side + i * step; });

            return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(range));
        }

        double builtin_random(const double &minimum_side, const double &maximum_side) {
            if (minimum_side > maximum_side) {
                throw std::runtime_error(
                    std::format("cannot generate number from [{}; {}]", minimum_side, maximum_side));
            }

            thread_local std::mt19937 generator(std::random_device{}());
            std::uniform_int_distribution<int> distribution(
                static_cast<int>(minimum_side),
                static_cast<int>(maximum_side)
            );

            return distribution(generator);
        }

        String builtin_time() {
            const auto current_time = std::chrono::system_clock::now();
            const auto time_t = std::chrono::system_clock::to_time_t(current_time);

            std::ostringstream output;
            output << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

            return std::make_shared<std::string>(output.str());
        }

        void builtin_sleep(const double &seconds) {
            std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        }

        void builtin_exit(const double &code) {
            std::cout << std::format("Finished with code {}\n", static_cast<int>(code));
            std::exit(static_cast<int>(code));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {"println", bind_function<builtin_println>("println")},
            {"readLine", bind_function<builtin_read_line>("readLine")},
            {"open", bind_function<builtin_open_file>("open")},
            {"range", bind_function<builtin_range>("range")},
            {"random", bind_function<builtin_random>("random")},
            {"time", bind_function<builtin_time>("time")},
            {"sleep", bind_function<builtin_sleep>("sleep")},
            {"exit", bind_function<builtin_exit>("exit")},
        };
    }
}
