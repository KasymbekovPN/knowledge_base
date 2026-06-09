---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::viewable_range` — concept из C++20 (`<ranges>`), который проверяет:

> можно ли безопасно преобразовать объект в view через `std::views::all()`.

# Зачем нужен

Ranges-алгоритмы и views часто хотят работать не только с уже готовыми `view`, но и с обычными контейнерами.

Например:

```cpp
std::vector<int> v{1,2,3};

auto r = std::views::all(v);
```

`views::all` превращает контейнер в объект, с которым удобно работать в ranges pipeline.

Для этого STL проверяет:

```cpp
std::ranges::viewable_range<T>
```

# Что проверяет concept

Упрощённо:

```cpp
std::ranges::range<T>
```

и дополнительно:

```text
из T можно получить view
```

через:

```cpp
std::views::all(t)
```

# Почему не используют просто range

Если написать:

```cpp
template<std::ranges::range R>
```

то внутри нельзя гарантировать, что:

```cpp
std::views::all(r)
```

будет корректным.

`viewable_range` именно это и гарантирует.

# Разница между range, view и viewable_range

### range

Есть:

```cpp
begin()
end()
```

### view

Range + лёгкое представление данных.

```cpp
std::views::iota(...)
```

### viewable_range

Range, который можно превратить в view.

```cpp
std::views::all(r)
```

# Сравнение

|Concept|Что гарантирует|
|---|---|
|`range`|есть begin/end|
|`view`|это лёгкое представление данных|
|`viewable_range`|можно безопасно получить view|

# Типичные примеры

|Тип|range|view|viewable_range|
|---|---|---|---|
|`std::vector<int>`|✅|❌|✅|
|`std::span<int>`|✅|✅|✅|
|`std::string_view`|✅|✅|✅|
|`std::views::iota(...)`|✅|✅|✅|

# Где используется

Практически все адаптеры views принимают именно:

```cpp
std::ranges::viewable_range
```

Например:

```cpp
std::views::filter
std::views::transform
std::views::take
std::views::drop
```

# Итог

`std::ranges::viewable_range` означает:

> "это диапазон, который можно безопасно передать в ranges pipeline и превратить в view через `std::views::all()`".

Именно поэтому в современном ranges-коде чаще встречается:

```cpp
template<std::ranges::viewable_range R>
```

чем просто:

```cpp
template<std::ranges::range R>
```

```cpp
#include <iostream>
#include <vector>
#include <ranges>

template<std::ranges::viewable_range R>
void test(R&& _input) {
    auto v = std::views::all(_input);
    for (auto item: v) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::vector<int>({1, 2, 3}));

    return 0;
}
```

```
1 2 3
```
