#include "engine/Driver/Driver.h"

void Driver::run() {
    while (true) {
        std::string prompt;
        if (!std::getline(std::cin, prompt)) {
            std::cerr << "\n";
            std::exit(EXIT_CODE_BROKEN_INPUT);
        }
    }
}

void Driver::run(const char* path) {
    auto executedFile = executeFile(path);
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