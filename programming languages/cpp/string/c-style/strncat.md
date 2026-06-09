---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

Функция `std::strncat(char *dest, const char *src, size_t n)` в C++ — это **безопасная версия функции `strcat`** , которая позволяет добавлять часть строки к другой, ограничивая количество копируемых символов.
- `dest` — указатель на целевой буфер (должен быть достаточно большим, чтобы вместить результат).
- `src` — строка, которую мы добавляем.
- `n` — максимальное количество символов для добавления из `src`.

> ⚠️ `strncat` добавляет **не более `n`** символов из `src`, **но не копирует завершающий `'\0'`** из `src`. Однако она **всегда добавляет свой собственный `'\0'` в конец результата** .

```cpp
#include <iostream>
#include <cstring>

int main() {
    char dest[11] = "01234";
    strncat(dest, "56789abc", 2);
    std::cout << "dest => " << dest << std::endl;

    return 0;
}
```

```
dest => 0123456
```
