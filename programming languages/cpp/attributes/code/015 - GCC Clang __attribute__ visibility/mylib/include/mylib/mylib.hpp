#pragma once

// Кроссплатформенный макрос экспорта символов.
// Windows (MSVC/MinGW): __declspec(dllexport|dllimport)
// Linux/macOS (GCC/Clang): __attribute__((visibility("default")))
#if defined(_WIN32) || defined(__CIGWIN)
    #if defined(MYLIB_EXPORTS)
        #define MYLIB_API __declspec(dllexport)
    #else
        #define MYLIB_API __declspec(dllimport)
    #endif
    #define MYLIB_HIDDEN
#else
    #if defined(MYLIB_EXPORTS)
        #define MYLIB_API __attribute__((visibility("default")))
    #else
        #define MYLIB_API
    #endif
    #define MYLIB_HIDDEN __attribute__((visibility("hidden")))
#endif

// Публичный класс библиотеки — часть ABI.
class MYLIB_API Calculator {
public:
    Calculator();

    [[nodiscard]] int add(int, int) const;
    [[nodiscard]] int multiply(int, int) const;

private:
    // ВАЖНЫЙ НЮАНС: методы класса, помеченного MYLIB_API, по умолчанию
    // наследуют visibility класса и ВСЁ РАВНО экспортируются, даже если
    // объявлены private — "private" это доступ на уровне C++ (компилятор),
    // а visibility — это видимость символа на уровне линкера/ABI, они
    // никак не связаны. Чтобы реально скрыть метод из экспортируемых
    // символов .so, нужно явно пометить его MYLIB_HIDDEN.
    MYLIB_HIDDEN [[nodiscard]] int internalHelper(int) const;
};

// Публичная свободная функция — тоже часть ABI.
MYLIB_API int freeFunctionAdd(int, int);

// Явно внутренняя функция: не экспортируется из .so вообще,
// даже если случайно попадёт в заголовок. Полезно для хелперов,
// которые должны быть недоступны через dlsym()/линковку извне,
// но по каким-то причинам не static и не в анонимном namespace.
MYLIB_HIDDEN void internalOnlyFunction();
