---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/str_literals/_|<=]]

- Хранится как последовательность байтов в **кодировке UTF-8**.
- Тип: `const char*`.
- Полезен для работы с юникод-текстом, особенно в кроссплатформенных приложениях.

```cpp
#include <windows.h>
#include <iostream>
#include <string>

int main() {
    SetConsoleOutputCP(CP_UTF8); // window
    // const char* utf8_str = u8"Привет, мир!"; // until С++20
    const char8_t* utf8_str = u8"Привет, мир!";
    std::cout
	    << "str <= " << reinterpret_cast<const  char*>(utf8_str)
	    << std::endl;

    return 0;
}
```

```
str <= Привет, мир!
```
