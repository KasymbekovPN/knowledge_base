---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

Стоит сначала развести два вопроса, которые легко смешать: сама reflection (`^^`, `std::meta::info`) и отдельная фича **annotations** (`[[=expr]]`) — они голосовались как разные предложения (P2996 и P3394), обе приняты в черновик C++26 на июньской встрече WG21 в Софии (2025).

**Базовая reflection закрывает половину задачи из `reflect-demo` — перечисление членов.** `^^T` даёт `std::meta::info` для типа, `nonstatic_data_members_of(^^T)` — список его полей, `identifier_of(member)` — имя, `[:member:]` (splice) — обращение к самому полю в generated-коде. Это то же самое, что раньше делал `libclang` снаружи компилятора — только теперь прямо внутри обычной `consteval`-функции:

```cpp
#include <meta>
#include <sstream>

template <typename T>
std::string to_json(const T& obj) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    template for (constexpr auto member : std::meta::nonstatic_data_members_of(^^T)) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << std::meta::identifier_of(member) << "\":" << obj.[:member:];
    }
    oss << "}";
    return oss.str();
}
```

Никакого отдельного шага генерации, никакого `libclang`, работает для любого `T` — это уже полностью заменяет "перечисление полей" из `[[clang::annotate("reflect:field")]]` + `generate.py`.

**Но вот "secret"-флаг у `salary` reflection сам по себе не решает** — то, какое поле секретное, не часть типа, это внешняя семантическая метка. Ровно для этого случая WG21 сознательно не стал переиспользовать обычные атрибуты, а ввёл **новый, отдельный синтаксис** — `[[=expr]]` вместо `[[attr]]`. Причина явно объясняется в самом P3394: атрибуты по стандарту обязаны быть _игнорируемыми_ компилятором, значит для них не гарантируется сохранение реального значения куда-либо, откуда reflection могла бы его прочитать; поэтому "annotation" — это отдельная конструкция, синтаксически похожая, но семантически другая: компайл-тайм значение структурного типа, которое **гарантированно** сохраняется и вытаскивается обратно через `std::meta::annotations_of()`.

Переписывая `reflect-demo` на этом (пример адаптирован из официальных примеров пейпера, экспериментально проверенных авторами на Compiler Explorer через форк Bloomberg — сам я собрать это в песочнице не могу, ни один доступный компилятор пока не реализует C++26 reflection):

```cpp
inline constexpr struct {} secret;

struct Person {
    int age;
    std::string name;
    [[=secret]] double salary;
};

template <typename T>
std::string to_json(const T& obj) {
    std::ostringstream oss;
    oss << "{";
    bool first = true;
    template for (constexpr auto m : std::meta::nonstatic_data_members_of(^^T)) {
        if (!first) oss << ",";
        first = false;
        oss << "\"" << std::meta::identifier_of(m) << "\":";
        if constexpr (std::meta::has_annotation<decltype(secret)>(m)) {
            oss << "\"***\"";
        } else {
            oss << obj.[:m:];
        }
    }
    oss << "}";
    return oss.str();
}
```

Это буквально тот же самый пайплайн, что я собирал через `libclang`, только весь целиком внутри стандартного C++, без внешнего инструмента, без отдельного шага сборки, переносимо между любыми компиляторами, реализовавшими C++26.

**Где пересечение реально "частичное", а не полное.** Мотивирующие примеры самого P3394 — это ровно те же классы задач, что обсуждались в этом чате: CLI-парсинг в духе Rust `clap` (аналог `_In_`/аргументов), параметризация тестов в духе pytest, сериализация в духе `serde` (то есть Qt moc/protobuf/serialization frameworks, о которых ты спрашивал изначально) — все три официальных мотивирующих примера пейпера. То есть reflection+annotations метит именно в этот use case: "прикрепить метаданные к объявлению, чтобы библиотека могла их прочитать во время компиляции и сгенерировать код".

Но атрибуты, которые обсуждались раньше в этом чате и меняют **кодогенерацию/ABI/линковку** — `visibility`, `packed`, `aligned`, `constructor`/`destructor`, `weak`, `section`, `noinline`/`always_inline`, `pure`/`const`, `nonnull`, `format` — это совсем другая категория: указания компилятору, как ему самому генерировать код или обращаться с символами, а не метаданные для стороннего кода. Reflection их не заменяет и не может заменить в принципе — они не про то, "что можно прочитать при компиляции", а про то, "что компилятор должен сделать с самим этим объявлением на уровне машинного кода". `[[nodiscard]]`, `[[deprecated]]` и подобные — тоже вне зоны пересечения, это диагностика компилятора, а не библиотечная метапрограммирование.

**Практический статус на сегодня (август 2026).** И `^^`/`std::meta`, и `[[=...]]` — экспериментальные фичи, реализованы только во форке Clang от Bloomberg (`-freflection-latest`) и в EDG frontend; ни в одном мейнстрим-релизе GCC, Clang или MSVC их пока нет. Это черновик C++26, а не то, что можно взять и использовать в проде прямо сейчас — практический смысл понимать это пересечение уже сегодня в том, чтобы не изобретать новый слой `[[clang::annotate(...)]]` + свой генератор для задач сериализации/CLI-парсинга/тестов, если через год-два эта задача решается стандартной библиотечной фичей без внешнего тулинга.

Sources:

- [Annotations for Reflection (P3394R0)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3394r0.html)
- [Reflection voted into C++26: "Whole new language" — Herb Sutter](https://isocpp.org/blog/2025/06/reflection-voted-into-cpp26-whole-new-language-herb-sutter)
- [Trip report: June 2025 ISO C++ standards meeting (Sofia, Bulgaria)](https://herbsutter.com/2025/06/21/trip-report-june-2025-iso-c-standards-meeting-sofia-bulgaria/)
- [P2996R13 — Reflection for C++26](https://isocpp.org/files/papers/P2996R13.html)

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
- [x] MSVC `__declspec(...)`: `novtable` (2026.08.06)
- [x] MSVC `__declspec(...)`: `restrict` (2026.08.06)
- [x] Как оборачивать их в кроссплатформенные макросы (`#if defined(__GNUC__) ...`). (2026.08.06)

**4. Атрибуты для оптимизации и кодогенерации** 
- [x]  `noinline`/`always_inline`, `pure`/`const` (GCC) — как они меняют ассемблерный вывод; проверка через Compiler Explorer (godbolt). (2026.08.06)

**5. Атрибуты для безопасности и статического анализа** 
- [x] `nonnull`, (2026.08.06)
- [x] `sentinel` (GCC),  (2026.08.06)
- [x] Clang thread-safety annotations (`__attribute__((guarded_by(...)))` и т.п.), (2026.08.06)
- [x] SAL-аннотации MSVC (`_In_`, `_Out_`, `_Ret_maybenull_`). (2026.08.06)

**6. Кастомные аннотации через атрибуты** 
- [x] `[[clang::annotate("...")]]` и `__attribute__((annotate("...")))` — как извлекать их через libclang/LLVM для генерации кода (аналог Qt moc, protobuf reflection, serialization frameworks). (2026.08.06)

**7. Будущее: атрибуты и рефлексия** 
- [x] Как C++26 static reflection (`^^`, `std::meta`) частично замещает use-case кастомных атрибутов — стоит понимать пересечение. (2026.08.06)

