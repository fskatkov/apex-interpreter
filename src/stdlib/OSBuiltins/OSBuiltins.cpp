#include "stdlib/OSBuiltins/OSBuiltins.h"

namespace stdlib::OSBuiltins {
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
            static Value invoke_implementation(auto func, const std::vector<Value> &args, std::index_sequence<Is...>) {
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
                resulting_arity = MethodTraits<decltype(Func)>::Arity;

                callable = [](const Value &, const std::vector<Value> &args) -> Value {
                    return MethodTraits<decltype(Func)>::invoke(Func, args);
                };
            }

            return std::make_shared<NativeFunction>(NativeFunction{
                .name = std::move(name),
                .arity = resulting_arity,
                .callable = std::move(callable)
            });
        }

        bool make_directory(const String &path, const String &directory) {
            const std::filesystem::path current_working_directory(*path);
            return std::filesystem::create_directory(current_working_directory / *directory);
        }

        void remove_directory(const String &path) {
            const std::filesystem::path current_working_directory(*path);
            std::filesystem::remove_all(current_working_directory);
        }

        String get_current_working_directory() {
            auto current_working_directory = std::filesystem::current_path().string();
            return std::make_shared<String::element_type>(current_working_directory);
        }

        void list_directory(const String &path) {
            const std::filesystem::path current_working_directory(*path);
            std::vector<std::string> directory_files;

            for (const auto &directory_file : std::filesystem::directory_iterator(current_working_directory)) {
                directory_files.push_back(directory_file.path().filename().string());
            }

            if (directory_files.empty()) return;

            std::ranges::sort(directory_files);

            const auto max_element = std::ranges::max_element(directory_files, {}, &std::string::length);
            const auto max_length = max_element->length();

            const auto number_of_columns = std::max<int>(1, 80 / (max_length + 2));
            const int number_of_rows = std::ceil(static_cast<double>(directory_files.size()) / number_of_columns);

            for (auto row = 0; row < number_of_rows; ++row) {
                for (auto column = 0; column < number_of_columns; ++column) {
                    if (const auto index = column * number_of_rows + row; index < directory_files.size()) {
                        std::cout << std::format("{:<{}}", directory_files[index], max_length + 2);
                    }
                }

                std::cout << "\n";
            }
        }

        double execute_system_command(const String &command) {
            return static_cast<double>(std::system(command->c_str()));
        }

        double get_process_id() {
            return static_cast<double>(getpid());
        }

        String get_platform_name() {
#if defined(_WIN32)
            return std::make_shared<String::element_type>("Windows");
#elif defined(__APPLE__) && defined(__MACH__)
            return std::make_shared<String::element_type>("macOS");
#elif defined(__linux__)
            return std::make_shared<String::element_type>("Linux");
#elif defined(__FreeBSD__)
            return std::make_shared<String::element_type>("FreeBSD");
#elif defined(__unix__)
            return std::make_shared<String::element_type>("UNIX");
#else
            return std::make_shared<String::element_type>("Unknown");
#endif
        }

        void builtin_exit(const double &code) {
            std::cout << std::format("Finished with code {}\n", static_cast<int>(code));
            std::exit(static_cast<int>(code));
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {"mkdir", bind_function<make_directory>("mkdir")},
            {"rmdir", bind_function<remove_directory>("rmdir")},
            {"cwd", bind_function<get_current_working_directory>("cwd")},
            {"ls", bind_function<list_directory>("ls")},
            {"system", bind_function<execute_system_command>("system")},
            {"getPID", bind_function<get_process_id>("getPID")},
            {"platform", bind_function<get_platform_name>("platform")},
            {"exit", bind_function<builtin_exit>("exit")},
        };
    }
}
