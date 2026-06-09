---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for functional/_|<=]]

`std::predicate` — concept из `<concepts>`, который описывает **предикат**.

Предикат — это вызываемый объект, который принимает аргументы и возвращает значение, приводимое к `bool`.
# Определение

Упрощённо стандарт определяет его так:

```cpp
template<class F, class... Args>
concept predicate =
    std::regular_invocable<F, Args...> &&
    std::boolean_testable<
        std::invoke_result_t<F, Args...>
    >;
```

То есть:
1. объект можно вызвать;
2. результат можно использовать в условии `if`.

# Связь с другими concepts

Иерархия выглядит так:

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
```

# Итог

`std::predicate<F, Args...>` гарантирует, что:
- `F` можно вызвать с аргументами `Args...`;
- результат вызова можно использовать как логическое значение;
- concept используется для фильтрации, поиска и проверки условий в алгоритмах STL и Ranges;
- является основой для более сложных concepts, таких как `std::relation` и `std::strict_weak_order`.

```cpp
#include <iostream>
#include <concepts>

bool is_possitive(const int x) {
    return x > 0;
}

auto&& is_negative = [](const int x) {
    return x < 0;
};

struct IsOdd {
    bool operator()(const int x) const {
        return x % 2 != 0;
    }
};

struct IsEven {
    int operator()(const int x) const {
        return x % 2 ? 0 : 1;
    }
};

template<typename F, typename... Args>
requires std::predicate<F, Args...>
void test(std::string&& lbl, F&& func, Args&&... args) {
    std::cout << lbl;
    if (func(args...)) {
        std::cout << " TRUE";
    } else {
        std::cout << " FALSE";
    }
    std::cout << std::endl;
}

int main() {
    test("is_possitive 1", is_possitive, 1);
    test("is_possitive -1", is_possitive, -1);

    test("is_negative 1", is_negative, 1);
    test("is_negative -1", is_negative, -1);

    test("IsOdd 1", IsOdd(), 1);
    test("IsOdd 2", IsOdd(), 2);

    test("IsEven 1", IsEven(), 1);
    test("IsEven 2", IsEven(), 2);

    return 0;
}
```

```
is_possitive 1 TRUE
is_possitive -1 FALSE
is_negative 1 FALSE
is_negative -1 TRUE
IsOdd 1 TRUE
IsOdd 2 FALSE
IsEven 1 FALSE
IsEven 2 TRUE
```
