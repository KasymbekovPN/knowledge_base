---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

Универсальный рецепт — один header с макросами, который прячет `#ifdef`, а остальной код их не видит вообще. Несколько важных моментов, прежде чем сам шаблон.

**Порядок проверки компиляторов.** Clang сам определяет `__GNUC__` (для совместимости с кодом, ожидающим GCC), поэтому если нужно различить именно Clang и именно GCC — проверяй `__clang__` первым. Если разница не нужна (оба понимают GNU-синтаксис `__attribute__`) — можно объединить их одной веткой:

```cpp
#if defined(__clang__)
    // Clang-специфичное (если вдруг нужно отличить от GCC)
#elif defined(__GNUC__)
    // "чистый" GCC
#endif

// но для большинства атрибутов достаточно одной проверки:
#if defined(__GNUC__) || defined(__clang__)
    // GNU-style __attribute__ — работает и на GCC, и на Clang, и на MinGW
#elif defined(_MSC_VER)
    // MSVC __declspec
#endif
```

**Лучше `__has_attribute`/`__has_cpp_attribute`, чем версия компилятора.** Это встроенные макросы-предикаты (поддерживаются GCC, Clang), которые проверяют конкретную возможность, а не гадают по номеру версии компилятора — надёжнее, потому что не все атрибуты появились одновременно во всех версиях:

```cpp
#if defined(__has_cpp_attribute)
  #if __has_cpp_attribute(nodiscard)
    #define NODISCARD [[nodiscard]]
  #endif
#endif
#ifndef NODISCARD
  #define NODISCARD
#endif

#if defined(__has_attribute)
  #if __has_attribute(always_inline)
    #define ALWAYS_INLINE __attribute__((always_inline)) inline
  #endif
#endif
#ifndef ALWAYS_INLINE
  #define ALWAYS_INLINE inline
#endif
```

**Общий принцип приоритета:** сначала стандартный `[[attribute]]`, если он есть для нужного C++ standard'а, — он одинаково работает везде и молча игнорируется неизвестными компиляторами. Только если стандартного эквивалента нет (`packed`, `visibility`, `constructor`, `weak`, `section`) — уходишь в `#ifdef` на конкретный компилятор.

Собранный пример — то, что по факту разворачивалось по кусочкам в этом чате (`MYLIB_API`, `NORETURN`, `NOVTABLE` и т.п.), сведённое в один header:

```cpp
#pragma once

// --- noreturn: стандартный атрибут есть с C++11, доп. ветки не нужны ---
#define NORETURN [[noreturn]]

// --- deprecated / nodiscard / maybe_unused: тоже чисто стандартные ---
#define DEPRECATED(msg) [[deprecated(msg)]]
#define NODISCARD       [[nodiscard]]
#define MAYBE_UNUSED    [[maybe_unused]]

// --- always_inline / noinline: стандартного аналога нет ---
#if defined(__GNUC__) || defined(__clang__)
  #define ALWAYS_INLINE __attribute__((always_inline)) inline
  #define NOINLINE      __attribute__((noinline))
#elif defined(_MSC_VER)
  #define ALWAYS_INLINE __forceinline
  #define NOINLINE      __declspec(noinline)
#else
  #define ALWAYS_INLINE inline
  #define NOINLINE
#endif

// --- экспорт символов из shared library ---
#if defined(_WIN32) || defined(__CYGWIN__)
  #if defined(BUILDING_DLL)
    #define API_EXPORT __declspec(dllexport)
  #else
    #define API_EXPORT __declspec(dllimport)
  #endif
#elif defined(__GNUC__) || defined(__clang__)
  #if defined(BUILDING_DLL)
    #define API_EXPORT __attribute__((visibility("default")))
  #else
    #define API_EXPORT
  #endif
#else
  #define API_EXPORT
#endif

// --- novtable: только MSVC, у GCC/Clang просто пусто ---
#if defined(_MSC_VER)
  #define NOVTABLE __declspec(novtable)
#else
  #define NOVTABLE
#endif

// --- weak symbol: только GCC/Clang, у MSVC аналога через declspec нет ---
#if defined(__GNUC__) || defined(__clang__)
  #define WEAK __attribute__((weak))
#else
  #define WEAK
#endif
```

Использование в коде — компилятор нигде не виден:

```cpp
class NOVTABLE IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual void execute() const = 0;
};

class API_EXPORT Calculator {
public:
    NODISCARD int add(int a, int b) const;
};

NORETURN void fail(const char* msg);

WEAK void onError(int code);
```

Практические грабли, на которые стоит обратить внимание:

- **Позиция атрибута отличается между стилями.** `__declspec` почти всегда ставится _перед_ именем/типом (`__declspec(dllexport) class Foo`), `[[attribute]]` и GNU `__attribute__` допускают больше позиций (до и после объявления), но не взаимозаменяемо для каждого конкретного атрибута — при написании кроссплатформенного макроса надо проверить оба варианта компиляции, а не полагаться на то, что раз скомпилировалось на одном компиляторе, скомпилируется и на другом.
- **`packed` — особый случай**, где семантика синтаксически различается сильнее: GCC/Clang ставят его на структуру целиком (`struct __attribute__((packed)) Foo {...}`), MSVC требует `#pragma pack(push, 1) ... #pragma pack(pop)` вокруг определения — макросом это не завернуть так же просто, как остальные, обычно делают отдельную пару макросов `PACKED_STRUCT_BEGIN`/`PACKED_STRUCT_END`.
- **Не всё нужно оборачивать.** Если пишешь код только под один компилятор (например, embedded-проект строго под GCC-тулчейн) — оборачивание в кроссплатформенные макросы просто добавляет лишний код без пользы; стоит делать это только когда реально нужна поддержка нескольких компиляторов/платформ, как было в demo-проектах выше (`visibility-demo`, `plugin-demo`).

**`__has_attribute(name)`** — проверяет поддержку **GNU-style** атрибута `__attribute__((name))`. Это расширение GCC/Clang, не часть стандарта C++. Возвращает `1`, если атрибут поддерживается, `0` — если нет (в макросах обычно проверяют просто как булево значение).

```cpp
#if defined(__has_attribute)
  #if __has_attribute(always_inline)
    #define ALWAYS_INLINE __attribute__((always_inline))
  #endif
#endif
```

**`__has_cpp_attribute(name)`** — проверяет поддержку атрибута в **стандартном синтаксисе** `[[name]]` или `[[vendor::name]]`. Сам `__has_cpp_attribute` официально стандартизирован в C++20 (как часть препроцессора, SD-6 feature-test), но большинство компиляторов поддерживали его как расширение ещё до этого, начиная с C++11/14 кода.

```cpp
#if defined(__has_cpp_attribute)
  #if __has_cpp_attribute(nodiscard)
    #define NODISCARD [[nodiscard]]
  #endif
#endif
```

Для vendor-namespace атрибутов имя пишется с `::`:

```cpp
#if __has_cpp_attribute(gnu::always_inline)
    // [[gnu::always_inline]] доступен
#endif
```

**Ключевое отличие в возвращаемом значении.** `__has_attribute` возвращает просто `1`/`0` — да/нет. `__has_cpp_attribute` для _стандартизированных_ атрибутов возвращает не `1`, а **дату принятия конкретного предложения** в стандарт (формат `YYYYMM`, как в SD-6 feature-test macros) — это позволяет отличить не просто "атрибут известен", а "известна именно та версия семантики, которая мне нужна":

```cpp
#if __has_cpp_attribute(nodiscard) >= 201907L
    // доступна форма C++20: [[nodiscard("reason")]]
    #define NODISCARD_MSG(msg) [[nodiscard(msg)]]
#elif __has_cpp_attribute(nodiscard) >= 201603L
    // только базовая форма C++17: [[nodiscard]], без сообщения
    #define NODISCARD_MSG(msg) [[nodiscard]]
#else
    #define NODISCARD_MSG(msg)
#endif
```

Для vendor-специфичных (не стандартизированных ISO) атрибутов вроде `gnu::always_inline` конкретное числовое значение не специфицировано стандартом — там обычно просто проверяют "истина/ложь" через `>= 1` или наличие макроса вообще, как и с `__has_attribute`.

Третий, менее известный родственник — **`__has_declspec_attribute(name)`** (Clang, для совместимости с `__declspec`-синтаксисом MSVC): проверяет `__declspec(name)`, то есть третий стиль атрибутов из тех, что обсуждались в этом чате.

```cpp
#if defined(__has_declspec_attribute)
  #if __has_declspec_attribute(novtable)
    #define NOVTABLE __declspec(novtable)
  #endif
#endif
```

Практический вывод: пиши проверку под тот синтаксис, который собираешься использовать — `[[...]]` → `__has_cpp_attribute`, `__attribute__((...))` → `__has_attribute`, `__declspec(...)` → `__has_declspec_attribute` (только Clang). Все три макроса нужно оборачивать в `#if defined(...)` перед использованием, потому что не каждый компилятор их вообще знает (например, старый GCC до определённой версии не понимал сам `__has_attribute` как встроенный оператор препроцессора — отсюда двойная защита `#if defined(__has_attribute)` снаружи).
