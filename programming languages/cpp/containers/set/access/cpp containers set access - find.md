---
tags:
  - programming-language
  - cpp
  - container
  - set
---
[[_cpp containers set - access|<=]]

Метод `find(const T& value)` возвращает итератор на элемент с указанным значением. Если элемент не найден, возвращает _end()_.

```cpp
#include <iostream>
#include <set>

void _test_find_result(const std::set<int>::iterator&,
					   const std::set<int>&);

int main() {
    std::set<int> s {1, 2, 3};
    _test_find_result(s.find(2), s);
    _test_find_result(s.find(42), s);

    return 0;
}

void _test_find_result(const std::set<int>::iterator &it,
					   const std::set<int> &set) {
    if (it == set.end()) {
        std::cout << "Absence";
    } else {
        std::cout << *it;
    }
    std::cout << std::endl;
}
```

```
2
Absence
```