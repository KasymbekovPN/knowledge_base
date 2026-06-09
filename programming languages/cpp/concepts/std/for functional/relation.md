---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for functional/_|<=]]

`std::relation` — concept из `<concepts>`, который описывает **бинарное отношение** между двумя типами.

Он проверяет, что некоторый вызываемый объект (обычно функция сравнения) является предикатом для всех комбинаций аргументов двух типов.

# Определение

Упрощённо:

```cpp
template<class R, class T, class U>
concept relation =
    std::predicate<R, T, T> &&
    std::predicate<R, U, U> &&
    std::predicate<R, T, U> &&
    std::predicate<R, U, T>;
```

То есть должны быть корректны вызовы:

```cpp
r(t, t)
r(u, u)
r(t, u)
r(u, t)
```

и результат каждого должен быть bool-подобным.

# Иерархия concepts

```text
invocable
    ↓
regular_invocable
    ↓
predicate
    ↓
relation
    ↓
equivalence_relation

relation
    ↓
strict_weak_order
```

# Аналог через requires

Можно представить `relation` так:

```cpp
template<typename R,
         typename T,
         typename U>
concept MyRelation =
requires(R r, T t, U u)
{
    { r(t, t) } -> std::convertible_to<bool>;
    { r(u, u) } -> std::convertible_to<bool>;
    { r(t, u) } -> std::convertible_to<bool>;
    { r(u, t) } -> std::convertible_to<bool>;
};
```

(стандартная версия дополнительно использует `std::predicate`).

# Итог
`std::relation<R, T, U>` гарантирует, что:
- `R` — вызываемый объект;
- результат вызова является bool-подобным;
- корректны все комбинации аргументов:
    - `(T, T)`
    - `(U, U)`
    - `(T, U)`
    - `(U, T)`
- concept используется как основа для отношений эквивалентности и отношений порядка (`strict_weak_order`).

```cpp
#include <iostream>
#include <concepts>

struct Compare {
    bool operator()(int a, double b) const { return a < b; }
    bool operator()(double a, int b) const { return a < b; }
    bool operator()(int a, int b) const { return a < b; }
    bool operator()(double a, double b) const { return a < b; }
};

template<
    typename T,
    typename U,
    std::relation<T, U> R
>
void test(T t, U u, R r) {
    std::cout
        << std::boolalpha
        << r(t, u)
        << std::noboolalpha
        << std::endl;
}

int main() {
    test(10, 20.0, std::ranges::less{});
    test(200.0, 42, Compare());

    return 0;
}
```

```
true
false
```
