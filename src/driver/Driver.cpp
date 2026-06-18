#include "driver/Driver.h"

int Driver::run(const int& argc, const char *argv[]) {
    if (argc == 1) {
        return execute(DriverConfig{
            DriverConfig::DriverMode::REPL_MODE,
            ""
        });
    }

    return execute(DriverConfig{
        DriverConfig::DriverMode::SCRIPT_MODE,
        argv[1]
    });
}

int Driver::execute(const DriverConfig &config) {
    switch (config.mode) {
        case DriverConfig::DriverMode::REPL_MODE: {
            DiagnosticEngine diagnosticEngine("");
            ExecutionEngine executionEngine(diagnosticEngine);

            while (true) {
                std::cout << "> ";

                std::string input;
                if (!std::getline(std::cin, input)) {
                    return 1;
                }

                const auto executedFile = executionEngine.run(input);
                if (executedFile == ExecutionResult::RUNTIME_ERROR) {
                    return 70;
                }

                if (executedFile == ExecutionResult::COMPILETIME_ERROR) {
                    return 65;
                }
            }
        }
        case DriverConfig::DriverMode::SCRIPT_MODE: {
            std::filesystem::path file_path(config.path);

            if (!std::filesystem::exists(file_path)) {
                std::cout << "\033[31m" << "apex: `" << config.path << "`: No such file or directory" << "\033[0m" << "\n";
                return 66;
            }

            if (file_path.extension() != ".apex") {
                std::cout << "\033[31m" << "error: Unsupported file type: " << config.path << "\033[0m" << "\n";
                return 66;
            }

            std::ifstream file(file_path, std::ios::in | std::ios::binary);
            if (!file.is_open()) {
                std::cout << "\033[31m" << "apex: `" << config.path << "`: Permission denied" << "\033[0m" << "\n";
                return 66;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            auto content = buffer.str();
            DiagnosticEngine diagnostic_engine(content, "<" + config.path + ">");
            ExecutionEngine execution_engine(diagnostic_engine);

            const auto executedFile = execution_engine.run(content);
            if (executedFile == ExecutionResult::RUNTIME_ERROR) {
                return 70;
            }

            if (executedFile == ExecutionResult::COMPILETIME_ERROR) {
                return 65;
            }

            break;
        }
    }
    return 0;
}