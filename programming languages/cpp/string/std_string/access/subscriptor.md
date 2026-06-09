---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/access/_|_]]

Доступ к i-му символу (без проверки границ). Попытка доступа к несуществующему элементу приводит к неопределенному поведению.

```cpp
#include <iostream>
#include <string>
  
int main() {
    const std::string str {"Hello"};
    for (auto &item: str) {
        std::cout << item << std::endl;
    }
    return 0;
}
```

```
H
e
l
l
o
```
