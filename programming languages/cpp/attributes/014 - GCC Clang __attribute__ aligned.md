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

---

План изучения атрибутов в C++ (для тех, кто уже знает язык):

**1. Стандартные атрибуты `[[attribute]]` (C++11 и новее)** Разберись по одному, с примерами и эффектом на кодогенерацию/варнинги:
- [x] `[[noreturn]]`, (2026.08.04)
- [x] `[[deprecated]]`, (2026.08.04)
- [x] `[[nodiscard]]`  (C++17, + `[[nodiscard("reason")]]` в C++20), (2026.08.04)
- [x] `[[maybe_unused]]`, (2026.08.04)
- [x] `[[fallthrough]]`, (2026.08.04)
- [x] `[[likely]]`/`[[unlikely]]` (C++20), (2026.08.04)
- [x] `[[carries_dependency]]`, (2026.08.04)
- [x] `[[no_unique_address]]` (C++20), (2026.08.04)
- [x] `[[assume]]` (C++23). (2026.08.04)

**2. Синтаксис и правила применения** 
- [x] Где атрибут можно ставить (перед объявлением, после имени, на тип, на statement), namespace-атрибуты (`[[gnu::...]]`, `[[msvc::...]]`), игнорирование неизвестных атрибутов компилятором (важное свойство стандартных атрибутов в отличие от компилятор-специфичных). (2026.08.04)

**3. Компилятор-специфичные расширения**

- [x] GCC/Clang `__attribute__((...))`: `always_inline` (2026.08.04)
- [x] GCC/Clang `__attribute__((...))`: `noinline` (2026.08.04)
- [x] GCC/Clang `__attribute__((...))`: `pure` (2026.08.04)
- [x] GCC/Clang `__attribute__((...))`: `const` (2026.08.04)
- [x] GCC/Clang `__attribute__((...))`: `packed` (2026.08.04)
- [ ] GCC/Clang `__attribute__((...))`: `aligned`
- [ ] GCC/Clang `__attribute__((...))`: `visibility`
- [ ] GCC/Clang `__attribute__((...))`: `constructor`/`destructor`
- [ ] GCC/Clang `__attribute__((...))`: `format`
- [ ] GCC/Clang `__attribute__((...))`: `weak`
- [ ] GCC/Clang `__attribute__((...))`: `section`
- [ ] MSVC `__declspec(...)`: `dllexport`/`dllimport`
- [ ] MSVC `__declspec(...)`: `align`
- [ ] MSVC `__declspec(...)`: `noreturn`
- [ ] MSVC `__declspec(...)`: `novtable`
- [ ] MSVC `__declspec(...)`: `restrict`
- [ ] Как оборачивать их в кроссплатформенные макросы (`#if defined(__GNUC__) ...`).

**4. Атрибуты для оптимизации и кодогенерации** 
- [ ]  `noinline`/`always_inline`, `pure`/`const` (GCC) — как они меняют ассемблерный вывод; проверка через Compiler Explorer (godbolt).

**5. Атрибуты для безопасности и статического анализа** 
- [ ] `nonnull`, 
- [ ] `sentinel` (GCC), 
- [ ] Clang thread-safety annotations (`__attribute__((guarded_by(...)))` и т.п.),
- [ ] SAL-аннотации MSVC (`_In_`, `_Out_`, `_Ret_maybenull_`).

**6. Кастомные аннотации через атрибуты** 
- [ ] `[[clang::annotate("...")]]` и `__attribute__((annotate("...")))` — как извлекать их через libclang/LLVM для генерации кода (аналог Qt moc, protobuf reflection, serialization frameworks).

**7. Будущее: атрибуты и рефлексия** 
- [ ] Как C++26 static reflection (`^^`, `std::meta`) частично замещает use-case кастомных атрибутов — стоит понимать пересечение.

Практика на каждом шаге: минимальные примеры + проверка эффекта в Compiler Explorer (GCC/Clang/MSVC), потому что многие атрибуты молча игнорируются, если написаны не там, где нужно — это главная ловушка при изучении.
