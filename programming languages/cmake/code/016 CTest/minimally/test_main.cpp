/*
cmake -B .build
cmake --build .build --config Debug
ctest --test-dir .build -C Debug
*/

#include "math.hpp"

#include <cassert>

int main() {
    assert(square(3) == 9);
    assert(square(0) == 0);
    assert(square(-4) == 16);

    return 0;
}
