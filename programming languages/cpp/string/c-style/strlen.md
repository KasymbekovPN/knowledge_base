---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/c-style/_|<=]]

`std::strlen(const char *str)` возвращает длину строки (без учёта завершающего нулевого символа `\0`).

Если в функцию `strlen` передать массив `char`, **не завершённый нулевым символом (`'\0'`)** , то поведение программы будет **неопределённым (undefined behavior, UB)** .

```cpp
#include <iostream>
#include <cstring>

int main() {
    const char str0[] = "Hello";
    const char str1[10] = "World";
    const char str2[] = {'!', '!', '!', '\0'};

    std::cout << "len of str0 <= " << strlen(str0) << std::endl;
    std::cout << "len of str1 <= " << strlen(str1) << std::endl;
    std::cout << "len of str2 <= " << strlen(str2) << std::endl;

    return 0;
}
```

```
len of str0 <= 5
len of str1 <= 5
len of str2 <= 3
```

```cpp
#include <iostream>
#include <cstring>

template <size_t N>
size_t get_array_size(const char (&)[N]) {
    return N;
}

int main() {
    const char line[] = {'!', '!', '!'};
    std::cout << "len <= " << get_array_size(line) << std::endl;

    return 0;
}
```

```
len <= 3
```
