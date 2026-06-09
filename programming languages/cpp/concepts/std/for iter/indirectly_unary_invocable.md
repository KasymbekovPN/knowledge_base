---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for iter/_|<=]]

`std::indirectly_unary_invocable` — concept из C++20 из `<iterator>`.

Он проверяет:
> можно ли вызвать функцию (callable) с результатом разыменования iterator-а.

# Идея

Concept проверяет примерно такое:

```cpp
func(*it)
```

# Что concept проверяет

Упрощенно:

```cpp
std::invoke(func, *it)
```

# Важный момент

Concept работает через:

```cpp
std::invoke
```

поэтому callable может быть:
- function
- lambda
- functor
- member function pointer
# Где используется

Concept применяется в:
- `std::ranges::for_each`
- `transform`
- `views`
- algorithms

# Реальный STL смысл

STL проверяет:

```text
можно ли применить функцию к элементу iterator-а
```

# Разница

| Concept                               | Проверяет |
| ------------------------------------- | --------- |
| `invocable<F, T>`                     | `f(t)`    |
| `indirectly_unary_invocable<F, Iter>` | `f(*it)`  |

# Иерархия

```text
indirectly_readable
        +
invocable
        ↓
indirectly_unary_invocable
```

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <concepts>

struct Iterator {
    int value{42};

    const int* operator*() const {
        return &value;
    }
};

void handle_int(int _input) {
    std::cout << "handle_int: " << _input << std::endl;
}

void handle_str(std::string _input) {
    std::cout << "handle_str: " << _input << std::endl;
}

struct Square {
    void operator()(int _input) const {
        std::cout << "sqr: " << _input * _input << std::endl;
    }
};

template<typename F, typename I>
requires std::indirectly_unary_invocable<F, I>
void test(F _func, I _it) {
    _func(*_it);
}

int main() {
    std::vector<int> v = {1, 2, 3};
    test(handle_int, v.begin());
    // test(handle_str, v.begin()); // Error

    Iterator it = Iterator();
    test(handle_int, *it);
    test(Square(), *it);

    return 0;
}
```


```
handle_int: 1
handle_int: 42
sqr: 1764
```
