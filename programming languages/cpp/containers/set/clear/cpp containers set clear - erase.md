---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - clear|<=]]

Метод `erase(const T& value)`  удаляет элемент с указанным значением. Возвращает количество удаленных элементов (0 или 1, так как элементы уникальны).

```cpp
#include <iostream>
#include <set>

template<class T>
void _test_erase(std::set<T>&, T);

int main() {
    std::set<int> s {1, 2, 3};
    _test_erase<int>(s, 2);
    _test_erase<int>(s, 42);

    return 0;
}

template<class T>
void _test_erase(std::set<T>& set, T element) {
    std::cout
        << element
        << " : "
        << std::boolalpha
        << set.erase(element)
        << std::noboolalpha
        << " : size "
        << set.size()
        << std::endl;
}
```

```
2 : 1 : size 2
42 : 0 : size 2
```
