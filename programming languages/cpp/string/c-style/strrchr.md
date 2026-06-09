---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strchr(const char *str, int ch)` ищет последний символ ch в строке str. Возвращает указатель на последний символ или nullptr.

```cpp
#include <iostream>
#include <cstring>

void _test_result(const char*, const char*);

int main() {
    const char *original = "Hello, world!!!";

    const char *found = strrchr(original, 'l');
    _test_result(found, original);

    const char *not_found = strrchr(original, 'a');
    _test_result(not_found, original);

    return 0;
}

void _test_result(const char* text_pointer, const char* original) {
    if (text_pointer) {
        std::cout
            << "Found at position: "
            << text_pointer - original
            << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }
}
```

```
Found at position: 10
Not found
```

