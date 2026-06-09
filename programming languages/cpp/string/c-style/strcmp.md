---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strcmp(const char *str1, const char *str2)` сравнивает две строки. Возвращает:
  - `0`, если строки равны,
  - отрицательное значение, если `str1 < str2`,
  - положительное значение, если `str1 > str2`.


```cpp
#include <iostream>
#include <cstring>

void _test_strcmp(const std::string&, const std::string&);

int main() {
    _test_strcmp("aaa", "aaa");
    _test_strcmp("aaa", "bbb");
    _test_strcmp("bbb", "aaa");

    return 0;
}

void _test_strcmp(const std::string& str1, const std::string& str2) {
    std::cout
        << "strcmp(" << str1 << ", "  << str2 << ") => "
        << strcmp(str1.c_str(), str2.c_str()) << std::endl;
}
```

```
strcmp(aaa, aaa) => 0
strcmp(aaa, bbb) => -1
strcmp(bbb, aaa) => 1
```
