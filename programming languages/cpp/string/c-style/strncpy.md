---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

В C++ (и в C) существует функция **`strncpy`** — это безопасная версия `strcpy`, которая позволяет копировать **ограниченное количество символов** из одной строки в другую. Однако важно понимать её поведение, потому что она **не всегда добавляет завершающий ноль (`'\0'`)** .

- Копирует **до `n` символов** из строки `src` в `dest`.
- Если строка `src` короче `n`, то оставшиеся символы заполняются нулями (`'\0'`).
- Если `src` длиннее `n`, то **`'\0'` не добавляется автоматически** .


```cpp
#include <iostream>
#include <cstring>

int main() {
    const char src[] = "0123456789";
    char dest_long[16];
    strncpy(dest_long, src, sizeof(dest_long) - 1);
    dest_long[sizeof(dest_long) - 1] = '\0';
    std::cout << "dest_long <= " << dest_long << std::endl;

    char dest_short[5];
    strncpy(dest_short, src, sizeof(dest_short) - 1);
    dest_short[sizeof(dest_short) - 1] = '\0';
    std::cout << "dest_short <= " << dest_short << std::endl;

    return 0;
}
```

```
dest_long <= 0123456789
dest_short <= 0123
```
