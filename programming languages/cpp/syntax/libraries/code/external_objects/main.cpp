#include <iostream>
#include "objects.h"

int main(int argc, char const *argv[]) {
    for (unsigned time {}; time < TIMES; time++) {
        std::cout << MESSAGE << std::endl;
    }

    for (unsigned time {}; time < TIMES; time++) {
        std::cout << C_MESSAGE << std::endl;
    }

    return 0;
}
