---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/compare/_|<=]]

Метод  `s.capacity()` возвращает текущий объём выделенной памяти

Метод  `s.reserve(N)` резервирует место под минимум __N__ символов

Метод  `s.shrink_to_fit()` уменьшает ёмкость до текущего размера (__C++11__)


```cpp
#include <iostream>
#include <string>

void _test_capacity(const std::string&);

int main() {
    std::string str {"Hello"};
    _test_capacity(str);

    str.reserve(32);
    _test_capacity(str);

    str.shrink_to_fit();
    _test_capacity(str);

    return 0;
}

void _test_capacity(const std::string& str) {
    std::cout
        << "capacity <= " << str.capacity()
        << " value <= " << str
        << std::endl;
}
```

```
capacity <= 15 value <= Hello
capacity <= 47 value <= Hello
capacity <= 15 value <= Hello
```
