---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for functional/_|<=]]

`std::invocable` — один из фундаментальных concepts из `<concepts>`.

Он проверяет:
> можно ли вызвать объект как функцию с указанными аргументами.

# Определение

Упрощённо:

```cpp
template<typename F, typename... Args>
concept invocable =
    requires(F&& f, Args&&... args)
    {
        std::invoke(
            std::forward<F>(f),
            std::forward<Args>(args)...
        );
    };
```

То есть concept проверяет, корректно ли выражение:

```cpp
std::invoke(f, args...)
```

# Что проверяет std::invoke

`std::invoke` умеет вызывать не только функции.

## Обычные функции

```cpp
void foo(int);

std::invoke(foo, 42);
```

## Лямбды

```cpp
std::invoke(lambda, 42);
```

## Функторы

```cpp
std::invoke(Adder{}, 1, 2);
```

## Методы класса

```cpp
struct X {
    void print() {}
};

static_assert(
    std::invocable<
        decltype(&X::print),
        X&
    >
);
```

Потому что допустимо:

```cpp
X x;

std::invoke(&X::print, x);
```

## Поля класса

```cpp
struct Person {
    int age;
};

static_assert(
    std::invocable<
        decltype(&Person::age),
        Person&
    >
);
```

Потому что:

```cpp
Person p{42};

std::invoke(&Person::age, p);
```

возвращает ссылку на поле.

# Связь с другими concepts

```text
invocable
    ↓
regular_invocable
```

А более специализированные concepts строятся поверх них:

```text
invocable
    ↓
predicate
    ↓
relation
    ↓
equivalence_relation
    ↓
strict_weak_order
```

# Итог

`std::invocable<F, Args...>` проверяет, что выражение

```cpp
std::invoke(f, args...)
```

корректно.

Подходят:
- обычные функции;
- лямбды;
- функторы;
- указатели на методы;
- указатели на поля класса.

Concept широко используется в обобщённом программировании для ограничения шаблонов, принимающих вызываемые объекты.

```cpp
#include <iostream>
#include <concepts>

int sum(int x, int y) {
    return x + y;
}

auto&& lambda = [](int x, int y) -> int {
    return x + y;
};

struct Sum {
    int operator()(int x, int y) const {
        return x + y;
    }
};

template<std::invocable<int, int>  F>
requires std::invocable<F, int, int>
void test(F func, int x, int y) {
    std::cout << func(x, y) << std::endl;
}

template<typename F, typename... Args>
requires std::invocable<F, Args...>
void test2(F&& func, Args&&... args) {
    std::cout << func(args...) << std::endl;
}

int main() {
    test(sum, 2, 3);
    test(lambda, 10, 20);
    test(Sum(), 42, 43);

    test2(Sum(), 142, 43);

    return 0;
}
```

```
5
30
85
185
```
