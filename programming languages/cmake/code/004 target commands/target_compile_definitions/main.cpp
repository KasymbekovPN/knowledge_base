/*
cmake -B .build-dbg
cmake --build .build-dbg --config Debug
./.build-dbg/app

cmake -B .build-rel
cmake --build .build-rel --config Release
./.build-rel/app
*/

#include <iostream>
#include <format>

int main() {
#ifdef ENABLE_LOGGING
    std::cout << "[LOG] Start\n";
#endif
    std::cout << std::format("Version: {}\n", APP_VERSION);
}
