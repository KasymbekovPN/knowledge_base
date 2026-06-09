---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Заменяет часть строки

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"__hello__"};
    std::cout << "str <= "<< str << std::endl;

    std::cout << "str <= "<< str.replace(2, 5, "world") << std::endl;
    std::cout << "str <= "<< str << std::endl;

    return 0;
}
```

```
str <= __hello__
str <= __world__
str <= __world__
```
