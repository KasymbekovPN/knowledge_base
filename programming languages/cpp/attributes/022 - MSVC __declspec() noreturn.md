---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__declspec(noreturn)` — MSVC-аналог `[[noreturn]]`/`__attribute__((noreturn))`: сообщает компилятору, что функция никогда не возвращает управление вызывающему коду (всегда бросает исключение, вызывает `abort`/`exit`, либо уходит в бесконечный цикл). Компилятор использует это для подавления предупреждений о недостижимом коде/отсутствии `return`, а также может немного эффективнее генерировать код после вызова такой функции.

```cpp
__declspec(noreturn) void fail(const char* msg) {
    throw std::runtime_error(msg);
}

int process(int x) {
    if (x < 0) {
        fail("negative value");   // MSVC не ждёт return после этого вызова
    }
    return x * 2;
}
```

По смыслу и последствиям нарушения (UB, если функция всё же вернётся) — то же самое, что обсуждалось в начале этого чата про `[[noreturn]]`. Практическая рекомендация: в новом C++ коде (C++11 и новее) почти всегда стоит использовать именно стандартный `[[noreturn]]` — он одинаково работает на MSVC, GCC и Clang, тогда как `__declspec(noreturn)` понимает только MSVC, а `__attribute__((noreturn))` — только GCC/Clang. Кроссплатформенный макрос для этого не нужен вообще, если не требуется поддержка компилятора до C++11:

```cpp
#if __cplusplus >= 201103L
  #define NORETURN [[noreturn]]
#elif defined(_MSC_VER)
  #define NORETURN __declspec(noreturn)
#elif defined(__GNUC__)
  #define NORETURN __attribute__((noreturn))
#else
  #define NORETURN
#endif
```

Единственный случай, где `__declspec(noreturn)` на практике всё ещё встречается — старый код на MSVC, писавшийся до C++11, или код, который должен компилироваться как чистый C на MSVC (где `[[noreturn]]` появился только в C23 и MSVC его поддержку добавил далеко не сразу, а C11 `_Noreturn` — отдельная история с собственными нюансами поддержки).

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
- [x] GCC/Clang `__attribute__((...))`: `aligned` (2026.08.04)
- [x] GCC/Clang `__attribute__((...))`: `visibility` (2026.08.05)
- [x] GCC/Clang `__attribute__((...))`: `constructor`/`destructor` (2026.08.05)
- [x] GCC/Clang `__attribute__((...))`: `format` (2026.08.05)
- [x] GCC/Clang `__attribute__((...))`: `weak` (2026.08.05)
- [x] GCC/Clang `__attribute__((...))`: `section` (2026.08.05)
- [x] MSVC `__declspec(...)`: `dllexport`/`dllimport` (2026.08.05)
- [x] MSVC `__declspec(...)`: `align` (2026.08.05)
- [x] MSVC `__declspec(...)`: `noreturn` (2026.08.06)
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
