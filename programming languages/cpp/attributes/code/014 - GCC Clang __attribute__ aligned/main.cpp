#include <format>
#include <iostream>
#include <format>

namespace {

    // объекты Vec4 всегда будут выровнены по 16 байт — удобно для SIMD (SSE/AVX загрузок)
    struct __attribute__((aligned(16))) Vec4 {
        float x, y, z, w;
    };

    // стандартный C++11 аналог через alignas
    alignas(16) float buffer0[4];
    // то же самое через GCC-атрибут
    __attribute__((aligned(16))) float buffer1[4];

    // aligned без аргумента — выравнивание по максимально "естественному" для платформы
    struct __attribute__((aligned)) MaxAligned { char c; };
}

int main() {
    std::cout << std::format("Vec4: {}\n", sizeof(Vec4));
    std::cout << std::format("buffer0: {}\n", sizeof(buffer0));
    std::cout << std::format("buffer1: {}\n", sizeof(buffer1));
    std::cout << std::format("MaxAligned: {}\n", sizeof(MaxAligned));

    return 0;
}
