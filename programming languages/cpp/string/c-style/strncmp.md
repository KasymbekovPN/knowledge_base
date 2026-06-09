---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strncmp(const char *str1, const char *str2, size_t n)` сравнивает первые `n` символов двух строк.

Возвращает:
  - `0`, если подстроки равны,
  - отрицательное значение, если `str1 < str2`,
  - положительное значение, если `str1 > str2`.

```cpp
#include <iostream>
#include <cstring>

void _test_strncmp(const std::string&, const std::string&, size_t);

int main() {
    _test_strncmp("aaa", "aaa", 3);
    _test_strncmp("aaa0", "aaa2", 3);
    _test_strncmp("aaa", "bbb", 3);
    _test_strncmp("bbb", "aaa", 3);

    return 0;
}

void _test_strncmp(const std::string& str1, 
				   const std::string& str2,
				   size_t len) {
    std::cout
        << "strcmp(" << str1 << ", "  << str2 << ") => "
        << strncmp(str1.c_str(), str2.c_str(), len) << std::endl;
}
```

```
strcmp(aaa, aaa) => 0
strcmp(aaa0, aaa2) => 0
strcmp(aaa, bbb) => -1
strcmp(bbb, aaa) => 1
```
