---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

В C++ (и в языке C) функция `strcat()` используется для **конкатенации (объединения)** строк в стиле C (`char[]` или `char*`). Однако **она не проверяет, достаточно ли места в целевом буфере** , и если места недостаточно — происходит **переполнение буфера** , что приводит к **неопределённому поведению** .

```cpp
#include <iostream>
#include <cstring>

int main() {
    char dest[16] = "Hello, ";
    strcat(dest, "world");
    std::cout << "dest => " << dest << std::endl;

    return 0;
}
```

```
dest => Hello, world
```
