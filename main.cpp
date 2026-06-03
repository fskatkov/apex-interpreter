#include <iostream>
#include "driver/Driver.h"

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        Driver::run();
    } else if (argc == 2) {
        Driver::run(argv[1]);
    }
    return 0;
}
