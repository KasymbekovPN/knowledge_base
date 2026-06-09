---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Вставляет текст в указанную позицию

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"____"};
    str.insert(2, "hello");
    std::cout << "str <= "<< str << std::endl;

    return 0;
}
```

```
str <= __hello__
```
