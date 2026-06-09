---
tags:
  - programming-language
  - cpp
  - syntax
  - library
  - include
---
[[__cpp syntax libraries__|<==]]

Кроме функций внешние файлы могут содержать различные объекты - переменные и константы. Для подключения внешних объектов в файл кода применяется ключевое слово _extern_.

Подключение констант имеет особенность - ключевое слово _extern_ надо указывать и при определении константы

__objects.h__
```cpp
#include <string>

extern unsigned TIMES;
extern std::string MESSAGE;
extern const std::string C_MESSAGE;
```

__objects.cpp__
```cpp
#include "objects.h"

unsigned TIMES {3};
std::string MESSAGE {"Hello"};
const std::string C_MESSAGE {"World"};
```

__main.cpp__
```cpp
#include <iostream>
#include "objects.h"

int main(int argc, char const *argv[]) {
    for (unsigned time {}; time < TIMES; time++) {
        std::cout << MESSAGE << std::endl;
    }

    for (unsigned time {}; time < TIMES; time++) {
        std::cout << C_MESSAGE << std::endl;
    }

    return 0;
}
```

```
clang++.exe .\main.cpp .\objects.cpp -o app.exe
```

```
Hello
Hello
Hello
World
World
World
```

---
[Внешние объекты](https://metanit.com/cpp/tutorial/3.9.php)