/*
clang++ -std=c++26 -x c++-module math.cppm --precompile -o math.pcm;
if ($?) { clang++ -std=c++26 -c math.pcm -o math.o }
if ($?) { clang++ -std=c++26 "-fmodule-file=math=math.pcm" -c demo.cpp -o demo.o }
if ($?) { clang++ math.o demo.o -o app.exe }
*/

import math;

#include <iostream>
#include <format>

int main() {
    int a{42};
    int b{12};
    std::cout << std::format("add({}, {}) = {}", a, b, add(a, b));

    return 0;
}