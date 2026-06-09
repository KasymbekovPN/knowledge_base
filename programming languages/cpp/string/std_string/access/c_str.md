---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/access/_|<=]]

Возвращает указатель на C-style строку (`const char*`), в случае пустой строке возвращает `\0`

```cpp
#include <iostream>
#include <string>

void _test_c_str(const std::string&);

int main() {
    const std::string str {"Hello"};
    const std::string empty_str {};
    _test_c_str(str);
    _test_c_str(empty_str);

    return 0;
}

void _test_c_str(const std::string& str) {
    std::cout << "First symbol <= '" << *str.c_str() << "'" << std::endl;
}
```

```
First symbol <= 'H'
First symbol <= ''
```
