#include "driver/Driver.h"

void Driver::run() {
    while (true) {
        std::cout << "> ";

        std::string prompt;

        if (!std::getline(std::cin, prompt)) {
            std::cerr << "\n";
            std::exit(EXIT_CODE_BROKEN_INPUT);
        }

        Scanner scanner(prompt);
        scanner.scan();

        if (scanner.encounteredErrors()) {
            scanner.raiseErrors();
            std::exit(EXIT_CODE_BROKEN_INPUT);
        }

        for (const auto& token : scanner.getTokens()) {
            std::cout << token.sourceLocation.line << ", " << token.sourceLocation.column << ", " << token.lexeme << "\n";
        }
    }
}

void Driver::run(const char* path) {
    auto executedFile = executeFile(path);

    Scanner scanner(executedFile);
    scanner.scan();

    if (scanner.encounteredErrors()) {
        const std::string strPath = path;
        scanner.raiseErrors(strPath);
        std::exit(EXIT_CODE_BROKEN_INPUT);
    }

    for (const auto& token : scanner.getTokens()) {
        std::cout << token.sourceLocation.line << ", " << token.sourceLocation.column << ", " << token.lexeme << "\n";
    }

}

std::string Driver::executeFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::string brokenPath = path;
        raiseError(EXIT_CODE_BROKEN_PATH,
          "can't open file `" + brokenPath + "`: [Error] No such file or directory\n");
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Driver::raiseError(const int& exitCode, const std::string& message) {
    std::cerr << message << "\n";
    std::exit(EXIT_CODE_BROKEN_PATH);
}