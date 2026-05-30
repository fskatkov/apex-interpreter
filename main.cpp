#include <iostream>
#include "engine/Driver/Driver.h"

int main(int argc, const char* argv[]) {
    Driver driver;
    if (argc == 1) {
        driver.run();
    } else if (argc == 2) {
        driver.run(argv[1]);
    }
    return 0;
}
