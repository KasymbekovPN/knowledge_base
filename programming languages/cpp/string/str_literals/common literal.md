---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/str_literals/_|<=]]

```cpp
#include <iostream>
#include <string>

int main() {
    const char* str = "hello";
    std::cout << "str <= " << str << std::endl;

    return 0;
}
```

```
str <= hello
```

- Представляет строку в виде массива `const char[]`.
- Завершается нулевым символом `\0`.
- Используется по умолчанию.
