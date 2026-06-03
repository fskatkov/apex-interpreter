#include "structures/TerminalController/TerminalController.h"

TerminalController::TerminalController() : idx(0) {
    enableRawMode();
}

TerminalController::~TerminalController() {
    disableRawMode();
}

std::string TerminalController::readLine(const std::string& input) {
    std::string buffer = "";
    idx = history.size();

    std::cout << input;
    std::cout.flush();

    while (true) {
        const auto key = readKey();

        if (key == static_cast<int>(KeyCode::ENTER)) {
            std::cout << "\n";
            if (!buffer.empty()) {
                history.push_back(buffer);
            }

            return buffer;
        }

        if (key == static_cast<int>(KeyCode::BACKSPACE)) {
            if (!buffer.empty()) {
                buffer.pop_back();
                clear(input);
                std::cout << buffer;
            }
        } else if (key == static_cast<int>(KeyCode::ARROW_UP)) {
            if (idx > 0) {
                idx--;
                buffer = history[idx];
                clear(input);
                std::cout << buffer;
            }
        } else if (key == static_cast<int>(KeyCode::ARROW_DOWN)) {
            if (idx < static_cast<int>(history.size() - 1)) {
                idx++;
                buffer = history[idx];
                clear(input);
                std::cout << buffer;
            } else {
                idx = history.size();
                buffer = "";
                clear(input);
            }
        } else if (key >= 32 && key <= 126) {
            buffer += static_cast<char>(key);
            std::cout << static_cast<char>(key);
        }

        std::cout.flush();
    }
}

void TerminalController::enableRawMode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void TerminalController::disableRawMode() const {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}

int TerminalController::readKey() {
    char symbol;
    if (read(STDIN_FILENO, &symbol, 1) != 1) {
        return static_cast<int>(KeyCode::UNKNOWN);
    }

    if (symbol == static_cast<char>(KeyCode::ESC)) {
        char sequence[3];
        if (read(STDIN_FILENO, &sequence[0], 1) != 1) {
            return static_cast<int>(KeyCode::ESC);
        }

        if (read(STDIN_FILENO, &sequence[1], 1) != 1) {
            return static_cast<int>(KeyCode::ESC);
        }

        if (sequence[0] == '[') {
            if (sequence[1] == 'A') {
                return static_cast<int>(KeyCode::ARROW_UP);
            }

            if (sequence[1] == 'B') {
                return static_cast<int>(KeyCode::ARROW_DOWN);
            }
        }

        return static_cast<int>(KeyCode::ESC);
    }

    return symbol;
}

void TerminalController::clear(const std::string& input) {
    std::cout << "\r\033[K" << input;
}
