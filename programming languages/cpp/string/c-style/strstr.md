---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strstr(const char *haystack, const char *needle)` ищет подстроку `needle` в строке `haystack`. Возвращает указатель на начало подстроки или `nullptr`.

```cpp
#include <iostream>
#include <cstring>

void _test_result(const char*, const char*);

int main() {
    const char *original = "Hello, world!!!";

    const char *found = strstr(original, "world");
    _test_result(found, original);

    const char *not_found = strstr(original, "abc");
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
Found at position: 7
Not found
```
