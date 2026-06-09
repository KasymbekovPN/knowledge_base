---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for ranges/_|<=]]

`std::ranges::view` — concept из C++20 (`<ranges>`), который обозначает:

> лёгкий range, который обычно не владеет данными и дешёв в копировании.

# Идея

Обычный контейнер:

```cpp
std::vector<int>
```

хранит данные.

View обычно хранит только:
- указатели
- итераторы
- ссылки
- параметры преобразования

и предоставляет "представление" данных.

# Что проверяет concept

Упрощённо:

```cpp
std::ranges::range<T>
```

и

```cpp
std::movable<T>
```

и

```cpp
std::ranges::enable_view<T>
```

# View и контейнер

Сравнение:

|Свойство|vector|view|
|---|---|---|
|Владеет данными|✅|обычно ❌|
|Хранит элементы|✅|обычно ❌|
|Дёшево копируется|❌|✅|
|Lazy вычисления|❌|часто ✅|

# Иерархия

```text
range
  ↓
view
```

Однако:

```text
view ⊂ range
```

то есть любой view — это range, но не любой range — view.

# Итог

`std::ranges::view` означает:
- объект является range;
- дёшево перемещается и копируется;
- обычно не владеет данными;
- используется для ленивых преобразований диапазонов;
- лежит в основе `std::views::filter`, `transform`, `take`, `drop`, `reverse`, `iota` и других компонентов ranges-библиотеки.

```cpp
#include <iostream>
#include <ranges>
#include <vector>

struct CustomView: public std::ranges::view_base {

private:
    int arr[3]{10, 20, 30};

public:
    int* begin() {
        return arr;
    }

    int *end() {
        return arr + 3;
    }
};

template<std::ranges::view T>
void test(T _view) {
    for (auto item: _view) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

int main() {
    test(std::views::iota(1, 4));

    std::vector<int> vec{1, 2, 3, 4, 5, 6, 7};
    test(vec | std::views::filter(
        [](int x) {return x % 2 == 0;}
    ));

    test(CustomView());

    return 0;
}
```

```
1 2 3 
2 4 6 
10 20 30
```

Самый простой способ — наследоваться от `std::ranges::view_base`.

# Почему работает

`view_base` автоматически включает:

```cpp
enable_view<MyView> == true
```
