#include "driver/Driver.h"

void Driver::run() {
    while (true) {
        std::cout << "> ";

        std::string prompt;
        if (!std::getline(std::cin, prompt)) {
            std::exit(65);
        }

        DiagnosticEngine diagnosticEngine(prompt);
        ExecutionEngine engine(prompt, diagnosticEngine);
        engine.run();
    }
}

void Driver::run(const char* path) {
    const std::string strPath = path;

    std::ifstream file(strPath);
    if (!file.is_open()) {
        std::cout << "can't open file `" + strPath + "`: [Error] No such file or directory\n";
        std::exit(66);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    auto executedFile = buffer.str();

    DiagnosticEngine diagnosticEngine(executedFile, strPath + ": ");
    ExecutionEngine engine(executedFile, diagnosticEngine);
    engine.run();
}