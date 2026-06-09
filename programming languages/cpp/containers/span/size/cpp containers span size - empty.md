---
tags:
  - programming-language
  - cpp
  - containers
---
[[_cpp containers span - size|<=]]

```cpp
#include <iostream>
#include <span>

template <typename T>
void _test_empty(std::span<T>&);

int main(int argc, char const *argv[]) {
    int array[] {1, 2, 3, 4, 5};
    std::span<int> not_empty_span {array};
    std::span<int> empty_span;

    _test_empty(not_empty_span);
    _test_empty(empty_span);

    return 0;
}

template <typename T>
void _test_empty(std::span<T>& spn) {
    std::cout
        << "Is it empty? "
        << std::boolalpha
        << spn.empty()
        << std::noboolalpha
        << std::endl;
}
```

```
Is it empty? false
Is it empty? true
```

---

### Размер и емкость
```cpp
// 1. size()/size_bytes()
size_t count = s.size();
size_t bytes = s.size_bytes();
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