/*
clang++ -std=c++23 --precompile math3.cppm -o math3.pcm;
if ($?) { clang++ -std=c++23 --precompile math4.cppm -o math4.pcm; }
if ($?) { clang++ -std=c++23 "-fmodule-file=math3=math3.pcm" "-fmodule-file=math4=math4.pcm" demo.cpp math3_impl.cpp math3.pcm math4.pcm -o app.exe }
*/

import math3;
import math4;

#include <iostream>
#include <format>

int main() {
    int a{42};
    int b{111};
    std::cout << std::format("add({}, {}) = {}\n", a, b , add(a, b));
    std::cout << std::format("sub({}, {}) = {}\n", a, b , sub(a, b));
    std::cout << std::format("add4({}, {}) = {}\n", a, b , add4(a, b));

    return 0;
}
