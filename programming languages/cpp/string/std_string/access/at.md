---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/access/_|<=]]

Доступ к i-му символу  с проверкой границ (бросает исключение `std::out_of_range`).

```cpp
#include <iostream>
#include <string>

void _test_at(const std::string&, size_t);

int main() {
    const std::string str {"Hello"};
    for (size_t i {}; i < str.size(); i++) {
        _test_at(str, i);
    }

    return 0;
}

void _test_at(const std::string& str, size_t idx) {
    try {
        std::cout << str.at(idx) << std::endl;
    } catch(const std::out_of_range& e) {
        std::cerr << e.what() << std::endl;
    }
}
```

```
H
e
l
l
o
```
