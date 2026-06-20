// clang++ -std=c++23 --precompile math.cppm -o math.pcm
// clang++ -std=c++23 -x c++-system-header --precompile iostream -o iostream.pcm
// clang++ -std=c++23 -x c++-system-header --precompile format -o format.pcm
// clang++ -std=c++23 "-fmodule-file=math=math.pcm" "-fmodule-file=iostream=iostream.pcm" "-fmodule-file=format=format.pcm" demo.cpp math.pcm iostream.pcm format.pcm -o demo.exe

import math;

#include <iostream>
#include <format>

int main() {
    std::cout << std::format("result: {}\n", add(2, 3));

    return 0;
}
