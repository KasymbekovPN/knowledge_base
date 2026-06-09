---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strcpy(char *dest, const char *src)` копирует строку src в dest, включая завершающий ноль.

⚠️ **Осторожно**: не проверяет размер буфера, может вызвать переполнение.

```cpp
#include <iostream>
#include <cstring>

int main() {
    const char src[] = "Hello";
    std::cout << "src <= " << src << std::endl;

    char dest[8];
    strcpy(dest, src);
    std::cout << "dest <= " << dest << std::endl;

    return 0;
}
```

```
src <= Hello
dest <= Hello
```
