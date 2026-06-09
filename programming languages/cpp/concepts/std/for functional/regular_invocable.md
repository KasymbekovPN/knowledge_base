---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for functional/_|<=]]

`std::regular_invocable` — concept из `<concepts>`, который является семантическим уточнением `std::invocable`.

Он означает:
> объект можно вызвать как функцию, и вызов должен вести себя как обычная функция: не зависеть от скрытого изменяемого состояния и не менять наблюдаемое состояние объекта.

# Определение

В стандарте:

```cpp
template<class F, class... Args>
concept regular_invocable =
    std::invocable<F, Args...>;
```

То есть формально проверка компилятором такая же, как у `std::invocable`.

# Важный момент

Для компилятора:

```cpp
std::regular_invocable<F, Args...>
```

и

```cpp
std::invocable<F, Args...>
```

обычно дают одинаковый результат.

Разница заключается в **семантике**, а не в синтаксисе.

# Отличие от invocable

### invocable

Проверяет только:

```cpp
f(args...)
```

компилируется.

### regular_invocable

Проверяет то же самое на уровне синтаксиса, но документирует дополнительное требование:

```text
вызов должен вести себя как обычная функция
```

# Таблица

| Concept             | Проверка компилятором | Семантическое требование           |
| ------------------- | --------------------- | ---------------------------------- |
| `invocable`         | вызов возможен        | нет                                |
| `regular_invocable` | вызов возможен        | должен вести себя как функция      |
| `predicate`         | вызов возможен        | возвращает bool-подобный результат |

# Иерархия

```text
invocable
    ↓
regular_invocable
    ↓
predicate
    ↓
relation
    ↓
strict_weak_order
```

# Итог

`std::regular_invocable<F, Args...>`:
- формально эквивалентен `std::invocable<F, Args...>`;
- проверяет, что объект можно вызвать;
- семантически предполагает, что вызов ведёт себя как обычная математическая функция;
- является базой для concepts `predicate`, `relation`, `strict_weak_order` и других алгоритмических ограничений STL.

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
    int value{};

    int operator()(int x, int y) {
        ++value;
        return x + y;
    }
};

template<typename F, typename... Args>
requires std::regular_invocable<F, Args...>
void test(F&& func, Args&&... args) {
    std::cout << func(args...) << std::endl;
}

int main() {
    test(sum, 2, 3);
    test(lambda, 10, 20);
    test(Sum(), 42, 43); // No error !!!

    return 0;
}
```

```
5
30
85
```

`test(Sum(), 42, 43); // No error !!!` - компилятор способен проверить, что функтор вызовется, но не умеет доказать математические свойства функции.
