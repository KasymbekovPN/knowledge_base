---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Заменяет содержимое строки

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"hello"};
    std::cout << "str <= " << str << std::endl;

    std::cout << "str <= " << str.assign("world") << std::endl;
    std::cout << "str <= " << str << std::endl;

    return 0;
}
```

```
str <= hello
str <= world
str <= world
```
