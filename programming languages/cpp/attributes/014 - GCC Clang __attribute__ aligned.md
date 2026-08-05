---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__attribute__((aligned(N)))` (`[[gnu::aligned(N)]]`) — задаёт минимальное выравнивание переменной, поля структуры или самого типа по границе `N` байт (обязательно степень двойки), в отличие от `packed`, который выравнивание убирает — `aligned` его, наоборот, усиливает (или явно фиксирует).

```cpp
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
```

Основные применения:

- **SIMD/векторизация** — SSE требует 16-байтного выравнивания, AVX — 32, AVX-512 — 64; невыровненные загрузки либо медленнее (movups vs movaps), либо падают на некоторых интринсиках.
- **Избежание false sharing** в многопоточном коде — выравнивание структуры по размеру cache line (обычно 64 байта), чтобы разные потоки не боролись за одну и ту же кэш-линию из-за соседних полей:

```cpp
struct alignas(64) PaddedCounter {
    std::atomic<int> value;
};
```

- **Требования конкретного железа/DMA-буферов** во embedded, где память должна начинаться по определённой границе.

Важно: с C++11 есть стандартный `alignas(N)` — это прямой, переносимый эквивалент `__attribute__((aligned(N)))`, и в современном C++ коде почти всегда стоит использовать именно его, а не GCC-специфичный атрибут. `__attribute__((aligned))` имеет смысл использовать только если нужна какая-то GCC/Clang-специфичная особенность (например, `aligned` без аргумента для "максимального" выравнивания, чего `alignas` без явного числа не делает) или в коде, который должен компилироваться и как C (где `alignas`/`_Alignas` появился только в C11 и не везде доступен).
