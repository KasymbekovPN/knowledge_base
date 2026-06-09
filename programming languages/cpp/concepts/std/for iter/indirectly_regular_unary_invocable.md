---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::indirectly_regular_unary_invocable` — concept из `<iterator>` (C++20), который проверяет:

> можно ли корректно вызвать функцию (callable) для значения, на которое указывает iterator, как для ссылки на элемент, так и для его value type.

Это более строгая версия `std::indirectly_unary_invocable`.

# Идея

Для iterator `I` и функции `F` должны быть корректны вызовы вида:

```cpp
f(*it)
```

и

```cpp
f(value)
```

где `value` имеет тип:

```cpp
std::iter_value_t<I>
```


# Связь с indirectly_unary_invocable

Есть два похожих concept-а:

| Concept                                   | Проверяет                                          |
| ----------------------------------------- | -------------------------------------------------- |
| `std::indirectly_unary_invocable`         | можно вызвать `f(*it)`                             |
| `std::indirectly_regular_unary_invocable` | можно вызвать и для `*it`, и для `iter_value_t<I>` |
|                                           |                                                    |

# Где используется

Этот concept применяется внутри ranges-алгоритмов:
- `std::ranges::transform`
- `std::ranges::for_each`
- различных views

Например STL хочет убедиться, что функция одинаково работает как с реальным элементом диапазона, так и с его копией.

```cpp
#include <iostream>
#include <vector>
#include <concepts>

struct Person {
    int age{42};

    const Person* operator*() const {
        return this;
    }
};

auto&& good_lambda = [](const Person& _p) {
    std::cout << "good_lambda: " << _p.age << std::endl;
};

auto&& bad_lambda = [](Person& _p) {
    std::cout << "bad_lambda: " << _p.age << std::endl;
};

void handle_int(int _input) {
    std::cout << "handle_int: " << _input << std::endl;
}

template<typename F, typename I>
requires std::indirectly_regular_unary_invocable<F, I>
void test(F _func, I _i) {
    _func(*_i);
}

int main(int argc, char const *argv[]) {
    std::vector<int> v {1, 2, 3};
    test(handle_int, v.begin());

    Person p = Person();
    test(good_lambda, *p);

    return 0;
}
```

```
handle_int: 1
goog_lambda: 42
```
