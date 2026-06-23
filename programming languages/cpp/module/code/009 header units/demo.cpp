/*
clang++ -std=c++23 -x c++-system-header --precompile iostream -o iostream.pcm;
if ($?) { clang++ -std=c++23 -x c++-system-header --precompile format -o format.pcm }
if ($?) { clang++ -std=c++23 -x c++-system-header --precompile vector -o vector.pcm }
if ($?) { clang++ -std=c++23 -x c++-header --precompile config.h -o config.pcm }
if ($?) { clang++ -std=c++23 "-fmodule-file=iostream=iostream.pcm" "-fmodule-file=format=format.pcm" "-fmodule-file=vector=vector.pcm" "-fmodule-file=config.h=config.pcm" demo.cpp iostream.pcm format.pcm vector.pcm config.pcm -o app.exe }
*/

import <iostream>;
import <format>;
import <vector>;
import "config.h";

int main() {
    std::vector<int> vec{1, 2, 3};
    std::cout << std::format("vec size {}\n", vec.size());

    std::cout << std::format("MAX_SIZE {}\n", MAX_SIZE);

    std::cout << std::format("numver {}\n", get_number());

    return 0;
}