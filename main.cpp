#include <iostream>
#include "REPL/REPL.h"

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        REPL::run();
    } else if (argc == 2) {
        REPL::run(argv[1]);
    }
    return 0;
}
