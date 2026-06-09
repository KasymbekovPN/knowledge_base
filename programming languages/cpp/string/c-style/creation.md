---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

Представляют собой массивы символов, завершающихся нулевым символом `'\0'`. 

В C++ для работы с **C-style строками** (нулевыми массивами символов `char*` или `char[]`) используется стандартная библиотека языка C — `<cstring>` (в C это `<string.h>`).

Эта библиотека предоставляет набор функций для манипуляции строками и копирования памяти. Ниже приведены **наиболее часто используемые методы из `<cstring>`**, а также примеры их применения.

```cpp
#include <iostream>

int main() {
    const char str0[] = "Hello";
    const char str1[10] = "World";
    const char str2[] = {'!', '!', '!', '\0'};
  
    std::cout << "str0 <= " << str0 << std::endl;
    std::cout << "str1 <= " << str1 << std::endl;
    std::cout << "str2 <= " << str2 << std::endl;

    return 0;
}
```

```
str0 <= Hello
str1 <= World
str2 <= !!!
```
