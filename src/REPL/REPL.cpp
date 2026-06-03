#include "REPL/REPL.h"

void REPL::run() {
    std::string buffer;

    while (true) {
        std::cout << "> ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }

        if (buffer.empty() && handleSpecialCommands(line)) continue;
        buffer += line + "\n";
        if (isIncompleteCommand(buffer)) continue;

        DiagnosticEngine diagnosticEngine(buffer);
        ExecutionEngine executionEngine(buffer, diagnosticEngine);
        executionEngine.run();
        buffer.clear();
    }
}

void REPL::run(const char* path) {
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

bool REPL::handleSpecialCommands(const std::string& command) {
    if (command == "exit") {
        std::exit(0);
    }

    if (command == "clear") {
        std::cout << "\033[2J\033[1;1H";
        return true;
    }

    return false;
}

bool REPL::isIncompleteCommand(const std::string& buffer) {
    int balance = 0;
    for (const auto& symbol : buffer) {
        if (symbol == '{' || symbol == '(') {
            balance++;
        } else if (symbol == '}' || symbol == ')') {
            balance--;
        }
    }
    return balance > 0;
}
