---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::common_range` — concept из C++20 (`<ranges>`), который проверяет:

> имеют ли `begin()` и `end()` один и тот же тип.

# Идея

Во многих старых контейнерах STL:

```cpp
auto b = r.begin();
auto e = r.end();
```

типы совпадают:

```cpp
decltype(b) == decltype(e)
```

Например:

```cpp
std::vector<int>::iterator
```

и

```cpp
std::vector<int>::iterator
```

# Определение

Упрощённо:

```cpp
template<class R>
concept common_range =
    std::ranges::range<R> &&
    std::same_as<
        std::ranges::iterator_t<R>,
        std::ranges::sentinel_t<R>
    >;
```

# Когда common_range = false

В ranges появились диапазоны, где конец диапазона представлен специальным sentinel.

Например:

```cpp
iterator != sentinel
```

но

```cpp
iterator
```

и

```cpp
sentinel
```

имеют разные типы.

# Почему sentinel полезен

Иногда конец диапазона невозможно представить обычным итератором.

Например:

```cpp
istream_view<int>
```

Чтение идёт до конца потока.
Там удобнее использовать специальный sentinel.

# Итог

`std::ranges::common_range` означает:
- диапазон является `range`;
- типы `iterator` и `sentinel` совпадают;
- `begin()` и `end()` возвращают один тип;
- примеры: `std::vector`, `std::array`, `std::span`, массивы;
- многие ленивые ranges используют отдельный sentinel и поэтому не являются `common_range`;
- для преобразования можно использовать `std::views::common`.

```cpp
#include <iostream>
#include <vector>
#include <ranges>

struct Sential {};

struct Iterator {
    using pointer = int*;
    using reference = int&;

    pointer ptr{nullptr};

    Iterator() = default;

    Iterator(pointer _p): ptr{_p} {}

    reference operator*() const {
        return *ptr;
    }

    Iterator& operator++() {
        ++ptr;
        return *this;
    }

    friend bool operator==(const Iterator& it, Sential) {
        return *it.ptr == -1;
    }
};

struct Range {
    Iterator begin() {
        return Iterator(new int{1});
    }

    Sential end() {
        return Sential();
    }
};

int main() {
    std::cout << std::ranges::common_range<std::vector<int>> << std::endl;
    std::cout << std::ranges::common_range<Range> << std::endl;

    return 0;
}
```

```
1
0
```
