#pragma once

#include "Common/Common.h"

class TerminalController {
public:
    explicit TerminalController();
    ~TerminalController();
    std::string readLine(const std::string& input);
private:
    enum class KeyCode {
        ENTER = 10,
        BACKSPACE = 127,
        ESC = 27,
        ARROW_UP = 1000,
        ARROW_DOWN = 1001,
        UNKNOWN = -1
    };

    struct termios original_termios;
    std::vector<std::string> history;
    int idx;

    void enableRawMode();
    void disableRawMode() const;
    static int readKey();
    static void clear(const std::string& input);
};
