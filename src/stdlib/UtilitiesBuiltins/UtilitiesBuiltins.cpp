#include "stdlib/UtilitiesBuiltins/UtilitiesBuiltins.h"

namespace stdlib::UtilitiesBuiltins {
    namespace {
        NIL builtin_println(const Value &, const std::vector<Value> &args) {
            for (const auto &arg: args) {
                std::cout << arg.str();
            }
            std::cout << std::endl;
            return NIL{};
        }

        String builtin_read_line(const Value &, const std::vector<Value> &args) {
            if (!args.empty() && args.front().is<String>()) {
                std::cout << *args.front().get<String>();
                std::cout.flush();
            }

            if (std::string input; std::getline(std::cin, input)) {
                return std::make_shared<std::string>(std::move(input));
            }

            throw std::runtime_error("invalid input format");
        }

        File builtin_open_file(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<String>()) {
                throw std::runtime_error(std::format("`{}`: No such file or directory", args.front().str()));
            }

            std::filesystem::path path(*args.front().get<String>());
            return std::make_shared<File::element_type>(path);
        }

        Array generate_range(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args.front().type()));
            }

            if (!args[1].is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args[1].type()));
            }

            const auto &start_side = args.front().get<double>();
            const auto &end_side = args[1].get<double>();

            if (start_side > end_side) {
                throw std::runtime_error(std::format("cannot generate sequence [{}; {})", start_side, end_side));
            }

            double step = 1;

            if (args.size() == 3) {
                if (!args[2].is<double>()) {
                    throw std::invalid_argument(std::format("number is required but got {}", args[1].type()));
                }

                step = args[2].get<double>();
            }

            auto sequence_length = static_cast<int>(std::floor(end_side - start_side) / step) + 1;
            auto range = std::views::iota(0, sequence_length)
                         | std::views::transform([=](const auto &i) { return start_side + i * step; });

            return std::make_shared<Array::element_type>(std::ranges::to<Array::element_type>(range));
        }

        double generate_random_number(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args.front().type()));
            }

            if (!args.back().is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args.back().type()));
            }

            const auto &minimum_side = static_cast<int>(args.front().get<double>());
            const auto &maximum_side = static_cast<int>(args.back().get<double>());

            if (minimum_side > maximum_side) {
                throw std::runtime_error(std::format("cannot generate random number from [{}; {}]", minimum_side, maximum_side));
            }

            std::random_device random_device;
            std::mt19937 generator(random_device());
            std::uniform_int_distribution<int> distribution(minimum_side, maximum_side);

            return distribution(generator);
        }

        String print_time(const Value &, const std::vector<Value> &args) {
            const auto current_time = std::chrono::system_clock::now();
            const auto time_t = std::chrono::system_clock::to_time_t(current_time);

            std::ostringstream output;
            output << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");

            return std::make_shared<std::string>(output.str());
        }

        NIL make_executable_sleep(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args.front().type()));
            }

            const std::chrono::duration<double> duration(args.front().get<double>());
            std::this_thread::sleep_for(duration);
            return NIL{};
        }

        NIL exit_program(const Value &, const std::vector<Value> &args) {
            if (!args.front().is<double>()) {
                throw std::invalid_argument(std::format("number is required but got {}", args.front().type()));
            }

            std::cout << "Finished with code " << std::to_string(static_cast<int>(args.front().get<double>())) << "\n";
            std::exit(static_cast<int>(args.front().get<double>()));
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
                "open", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "open",
                    .arity = 1,
                    .callable = builtin_open_file
                })
            },
            {
                "range", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "range",
                    .arity = -1,
                    .callable = generate_range
                })
            },
            {
                "random", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "random",
                    .arity = 2,
                    .callable = generate_random_number
                })
            },
            {
                "time", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "time",
                    .arity = 0,
                    .callable = print_time
                })
            },
            {
                "sleep", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "sleep",
                    .arity = 1,
                    .callable = make_executable_sleep
                })
            },
            {
                "exit", std::make_shared<NativeFunction>(NativeFunction{
                    .name = "exit",
                    .arity = 1,
                    .callable = exit_program
                })
            },
        };
    }
}
