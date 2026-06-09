---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - access|<=]]

Метод `last()` возвращает новый `span`, содержащий последние `N` элементов исходного диапазона. Это удобный способ получить "хвост" последовательности без необходимости вычисления индексов вручную.

## Особенности
1. **Границы**: Если запросить больше элементов, чем есть в `span`, поведение не определено (UB)
2. **Статические экстенты**: Для `span` с известным на этапе компиляции размером (`span<T, N>`) есть дополнительные проверки
3. **Невладеющий**: Результат `last()` не владеет данными, только ссылается на часть исходного `span`
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

    auto sub_span = original.last(3);
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
{3, 4, 5}
```

---

## Основные методы

### Доступ к элементам
```cpp
// 3. subspan() - произвольный диапазон
auto middle = s.subspan(1, 3);  // начиная с 1, длина 3

// 4. data() - получение указателя
int* ptr = s.data();
```


### Размер и емкость
```cpp
// 1. size()/size_bytes()
size_t count = s.size();
size_t bytes = s.size_bytes();

// 2. empty()
bool is_empty = s.empty();
```



## Расширенные возможности (C++23)

1. **Конструирование из временных объектов**:
```cpp
std::span s = std::vector{1, 2, 3};  // Опасность! Временный объект
```

2. **Методы для работы с байтами**:
```cpp
std::span<std::byte> bytes = std::as_writable_bytes(s);
```

3. **Статические экстенты**:
```cpp
std::span<int, 3> fixed_size(arr);  // Размер известен в compile-time
```

`std::span` - это мощный инструмент для работы с непрерывными данными, который сочетает в себе безопасность и производительность.