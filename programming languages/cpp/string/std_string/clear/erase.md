---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/clear/_|<=]]

Удаляет `len` символов начиная с позиции `pos`

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"0123456789"};
    std::cout << "str <= " << str << std::endl;

    str.erase(2, 5);
    std::cout << "str <= " << str << std::endl;

    return 0;
}
```

```
str <= 0123456789
str <= 01789
```
