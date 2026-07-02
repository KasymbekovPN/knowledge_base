/*
cmake -B .build -DCMAKE_PREFIX_PATH="C:\projects\knowledge_base\programming languages\cmake\code\015 target export\lib\install"
cmake --build .build --config Release
*/
#include <myLib/greeter.hpp>

#include <iostream>

int main() {
    auto s = std::string("WORLD");
    std::cout << greet(s) << std::endl;
}
