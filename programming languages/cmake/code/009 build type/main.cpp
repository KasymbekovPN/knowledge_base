/*

cmake -B .build-dbg
cmake --build .build-dbg --config Debug --verbose

cmake -B .build-rel
cmake --build .build-rel --config Release --verbose
*/

#include <iostream>

int main() {
    std::cout << "Hello World!\n";
#ifdef ENABLE_ASSERTS
    std::cout << "ENABLE_ASSERTS\n";
#endif
#ifdef VERBOSE_LOG
    std::cout << "VERBOSE_LOG\n";
#endif
#ifdef NDEBUG
    std::cout << "NDEBUG\n";
#endif

    return 0;
}
