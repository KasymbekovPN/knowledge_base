---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/size/_|<=]]

Возвращает длину строки (без `\0`)

```cpp
#include <iostream>
#include <string>

void _test_size(const std::string&);

int main() {
    const std::string empty {};
    const std::string not_empty {"hello"};

    _test_size(empty);
    _test_size(not_empty);

    return 0;
}

void _test_size(const std::string& str) {
    std::cout
        << "Size => "
        << str.size()
        << std::endl;
}
```

```
Size => 0
Size => 5
```
