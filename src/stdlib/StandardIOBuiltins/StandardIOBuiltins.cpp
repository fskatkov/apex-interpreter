#include "stdlib/StandardIOBuiltins/StandardIOBuiltins.h"

namespace stdlib::StandardIOBuiltins {
    namespace {
        Value builtin_print(const Value &, const std::vector<Value> &args) {
            for (const auto &arg: args) {
                std::cout << arg.str();
            }
            return NIL{};
        }

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

            return NIL{};
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
            }
        };
    }
}
