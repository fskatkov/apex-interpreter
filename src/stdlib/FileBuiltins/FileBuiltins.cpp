#include "stdlib/FileBuiltins/FileBuiltins.h"

namespace stdlib::FileBuiltins {
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

        template<typename R, typename... Args, R(*Func)(const File &, Args...)>
        struct MethodTraits<Func> {
            static constexpr int Arity = sizeof...(Args);

            static Value invoke(Value receiver, const std::vector<Value> &args) {
                const auto &file = receiver.get<File>();

                if (args.size() != Arity) [[unlikely]] {
                    throw std::invalid_argument(std::format("expected {} arguments, but got {}", Arity, args.size()));
                }

                return invoke_implementation(file, args, std::make_index_sequence<Arity>{});
            }

        private:
            template<std::size_t... Is>
            static Value invoke_implementation(const File &file, const std::vector<Value> &args,
                                               std::index_sequence<Is...>) {
                if constexpr (std::is_void_v<R>) {
                    Func(file, get_from_value<Args>(args[Is])...);
                    return NIL{};
                } else {
                    return convert_to_value(Func(file, get_from_value<Args>(args[Is])...));
                }
            }
        };

        template<auto Func>
        std::pair<std::string, std::shared_ptr<NativeFunction>> bind_method(const std::string &name) {
            return {
                name,
                std::make_shared<NativeFunction>(NativeFunction{
                    .name = name,
                    .arity = MethodTraits<Func>::Arity,
                    .callable = MethodTraits<Func>::invoke
                })
            };
        }

        String check_file_extension(const File &file) {
            return std::make_shared<String::element_type>(file->path.extension().string());
        }

        String check_file_permissions(const File &file) {
            auto permissions = std::filesystem::status(file->path).permissions();

            std::string resulting_permissions;

            auto check_permission = [&permissions, &resulting_permissions](const std::filesystem::perms bit, const char symbol) {
                resulting_permissions += std::to_underlying(permissions & bit) != 0 ? symbol : '-';
            };

            check_permission(std::filesystem::perms::owner_read, 'r');
            check_permission(std::filesystem::perms::owner_write, 'w');
            check_permission(std::filesystem::perms::owner_exec, 'x');

            check_permission(std::filesystem::perms::group_read, 'r');
            check_permission(std::filesystem::perms::group_write, 'w');
            check_permission(std::filesystem::perms::group_exec, 'x');

            check_permission(std::filesystem::perms::others_read, 'r');
            check_permission(std::filesystem::perms::others_write, 'w');
            check_permission(std::filesystem::perms::others_exec, 'x');

            return std::make_shared<String::element_type>(resulting_permissions);
        }

        String read_file_content(const File &file) {
            if (file->is_file_closed) {
                throw std::runtime_error(std::format("cannot read a closed file at `{}`", file->path.string()));
            }

            std::ifstream content(file->path, std::ios::in | std::ios::binary);
            if (!content) {
                throw std::runtime_error(std::format("file at `{}` is not available for reading", file->path.string()));
            }

            std::ostringstream buffer;
            buffer << content.rdbuf();

            return std::make_shared<String::element_type>(buffer.str());
        }

        void append_content_to_end_of_file(const File &file, const String &content) {
            if (file->is_file_closed) {
                throw std::runtime_error(std::format("cannot append to a closed file at `{}`", file->path.string()));
            }

            std::ofstream output(file->path, std::ios_base::app);

            if (!output) {
                throw std::runtime_error(std::format("file at `{}` is not available for editing", file->path.string()));
            }

            output << *content;
        }

        void write_content_to_file(const File &file, const String &content) {
            if (file->is_file_closed) {
                throw std::runtime_error(std::format("cannot write to a closed file at `{}`", file->path.string()));
            }

            std::ofstream output(file->path, std::ios_base::out | std::ios_base::trunc);

            if (!output) {
                throw std::runtime_error(std::format("file at `{}` is not available for editing", file->path.string()));
            }

            output << *content;
        }

        void remove_file(const File &file) {
            if (file->is_file_closed) {
                throw std::runtime_error(std::format("cannot remove a closed file at `{}`", file->path.string()));
            }

            std::filesystem::remove(file->path);
            file->is_file_closed = true;
        }

        void clear_file(const File &file) {
            if (file->is_file_closed) {
                throw std::runtime_error(std::format("cannot clear a closed file at `{}`", file->path.string()));
            }

            std::ofstream output(file->path, std::ios_base::out | std::ios_base::trunc);
        }

        void close_file(const File &file) {
            file->is_file_closed = true;
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            bind_method<check_file_extension>("extension"),
            bind_method<check_file_permissions>("permissions"),
            bind_method<read_file_content>("read"),
            bind_method<append_content_to_end_of_file>("append"),
            bind_method<write_content_to_file>("write"),
            bind_method<remove_file>("remove"),
            bind_method<clear_file>("clear"),
            bind_method<close_file>("close")
        };
    }
}
