---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::borrowed_range` — concept из C++20 из `<ranges>`.

Он проверяет:
> безопасно ли использовать iterator-ы после уничтожения range object.

# Главная идея

Некоторые ranges возвращают iterator-ы,  
которые становятся dangling после уничтожения range.

`borrowed_range` говорит: iterator живет независимо от объекта range

# STL включает borrowed_range для:
- `std::span`
- `std::string_view`
- `std::ranges::subrange`
- references to ranges

# Почему span — borrowed_range

`std::span` НЕ владеет памятью. Iterator указывает напрямую на массив.

А вот vector — НЕ borrowed_range

# Разница

|Concept|Что гарантирует|
|---|---|
|`range`|есть begin/end|
|`borrowed_range`|iterator переживает range|

# Когда использовать

Только если iterator действительно независим от lifetime объекта.
# Где используется

Concept активно применяется в:
- ranges algorithms
- views
- lazy pipelines

# Важный нюанс

Reference всегда borrowed:

```cpp
std::ranges::borrowed_range<
    std::vector<int>&
>
```

== true.
Сам объект vector живет снаружи.

```cpp
#include <iostream>
#include <ranges>
#include <vector>
#include <concepts>
#include <span>

struct Range {
    int arr[3]{1, 2, 3};

    int* begin() {
        return arr;
    }

    int* end() {
        return arr + 3;
    }
};

template<>
inline constexpr bool
    std::ranges::enable_borrowed_range<Range> = true;

template<std::ranges::borrowed_range T>
void test(T& range) {
    for (auto& i: range) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

int main() {
    int arr[]{100, 101, 102};
    auto&& s = std::span<int>(arr);
    test(s);

    auto&& range = Range();
    test(range);

    auto&& vec = std::vector({42, 43});
    // test(vec); // Error

    return 0;
}
```

```
100 101 102 
1 2 3
```

# Что проверяет concept

Упрощенно:

```cpp
enable_borrowed_range<T> == true
```

# Почему это опасно

Вы вручную обещаете STL:

```text
iterator не станет dangling
```
