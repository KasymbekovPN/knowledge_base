---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - access|<=]]

Метод `first()` позволяет получить новый `span`, представляющий первые N элементов исходного диапазона.

## Особенности

1. **Невалидные запросы**: Если запросить больше элементов, чем есть в `span`, поведение не определено (UB)
2. **Статические экстенты**: Для `span` с известным на этапе компиляции размером (`span<T, N>`) есть дополнительные проверки
3. **Невладеющий**: Результат `first()` не владеет данными, а только ссылается на часть исходного `span`
4. **Производительность**: Не копирует данные, только создаёт новую "видимость"


```cpp
#include <iostream>
#include <span>

template <typename T>
void _print_span(const std::span<T>&);

int main(int argc, char const *argv[]) {
    int array[] {1, 2, 3, 4, 5};
    std::span<int> original {array};
    _print_span(original);

    auto sub_span = original.first(3);
    _print_span(sub_span);

    return 0;
}

template <typename T>
void _print_span(const std::span<T>& s) {
    std::cout << "{";
    std::string delimiter = "";
    for (int &item: s) {
        std::cout << delimiter << item;
        delimiter = ", ";
    }
    std::cout << "}" << std::endl;
}
```

```
{1, 2, 3, 4, 5}
{1, 2, 3}
```
