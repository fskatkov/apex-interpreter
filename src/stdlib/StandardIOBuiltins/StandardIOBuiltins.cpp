#include "stdlib/StandardIOBuiltins/StandardIOBuiltins.h"

namespace stdlib::StandardIOBuiltins {
    namespace {
        Value builtin_println(const Value &, const std::vector<Value> &args) {
            for (const auto &arg: args) {
                std::cout << arg.str();
            }
            std::cout << std::endl;
            return NIL{};
        }

        Value builtin_read_line(const Value &, const std::vector<Value> &args) {
            if (!args.empty() && args.front().is<String>()) {
                std::cout << *args.front().get<String>();
                std::cout.flush();
            }

            if (std::string input; std::getline(std::cin, input)) {
                return std::make_shared<std::string>(std::move(input));
            }

            throw std::runtime_error("invalid input format");
        }

        Value builtin_flush(const Value &, const std::vector<Value> &args) {
            std::cout.flush();
            return NIL{};
        }

        Value builtin_close(const Value &, const std::vector<Value> &args) {
            std::cin.rdbuf(nullptr);
            std::cout.rdbuf(nullptr);
            return NIL{};
        }

        String builtin_open_file(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                throw std::invalid_argument("did not provide file path");
            }

            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            std::filesystem::path path(*args.front().get<String>());

            if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
                throw std::runtime_error("file not found or is a directory");
            }

            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file) {
                throw std::runtime_error(std::format("failed to read file at path `{}`", path.string()));
            }

            std::ostringstream buffer;
            buffer << file.rdbuf();

            return std::make_shared<std::string>(buffer.str());
        }

        bool builtin_check_file_exists(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                throw std::invalid_argument("did not provide file path");
            }

            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            const std::filesystem::path path(*args.front().get<String>());
            return std::filesystem::exists(path);
        }

        NIL builtin_write_to_file(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            if (!args[1].is<String>()) {
                throw std::invalid_argument("invalid data");
            }

            const std::filesystem::path path(*args.front().get<String>());
            if (std::ofstream file(path, std::ios::out | std::ios::trunc | std::ios::binary); file) {
                file << *args[1].get<String>();
            }

            return NIL{};
        }

        NIL builtin_append_to_file(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            if (!args[1].is<String>()) {
                throw std::invalid_argument("invalid data");
            }

            const std::filesystem::path path(*args.front().get<String>());
            if (std::ofstream file(path, std::ios::out | std::ios::app | std::ios::binary); file) {
                file << *args[1].get<String>();
            }

            return NIL{};
        }

        bool builtin_remove_file(const Value &, const std::vector<Value> &args) {
            if (args.empty()) {
                throw std::invalid_argument("did not provide file path");
            }

            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            const std::filesystem::path path(*args.front().get<String>());

            std::error_code error_code;
            const auto &removed = std::filesystem::remove(path, error_code);
            return removed;
        }
    }

    std::unordered_map<std::string, std::shared_ptr<NativeFunction> > register_methods() {
        return {
            {
                "println", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "println",
                    .arity = -1,
                    .callable = builtin_println
                })
            },
            {
                "readLine", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "readLine",
                    .arity = -1,
                    .callable = builtin_read_line
                })
            },
            {
                "flush", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "flush",
                    .arity = 0,
                    .callable = builtin_flush
                })
            },
            {
                "close", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "close",
                    .arity = 0,
                    .callable = builtin_close
                })
            },
            {
                "open", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "open",
                    .arity = 1,
                    .callable = builtin_open_file
                })
            },
            {
                "fileExists", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "fileExists",
                    .arity = 1,
                    .callable = builtin_check_file_exists
                })
            },
            {
                "write", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "write",
                    .arity = 2,
                    .callable = builtin_write_to_file
                })
            },
            {
                "append", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "append",
                    .arity = 2,
                    .callable = builtin_append_to_file
                })
            },
            {
                "removeFile", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "removeFile",
                    .arity = 1,
                    .callable = builtin_remove_file
                })
            },
        };
    }
}
