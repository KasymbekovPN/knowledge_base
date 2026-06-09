---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`strcpy_s(char *dest, size_t destSize, const char *src)` — **безопасная** версия, добавленная позже как часть расширения безопасности (из стандарта **C11 Annex K** ) и доступна не во всех компиляторах.

- Поддерживается MSVC (Visual Studio).
- **GCC / Clang** **не поддерживают `strcpy_s` напрямую** , но могут поддерживать аналоги (`strlcpy`, или через флаги).

```cpp
#include <iostream>
#include <cstring>

int main() {
    char src[] = "Hello world!!!";
  
    char dest0[10];
    errno_t result0 = strcpy_s(dest0, sizeof(dest0), src);
    if (result0 == 0) {
        std::cout << "[0+] " << dest0 << std::endl;
    } else {
        std::cout << "[0-]" << std::endl;
    }

    return 0;
}
```
