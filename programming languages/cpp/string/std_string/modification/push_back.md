---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Добавляет один символ в конец

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"a"};
    std::cout << "str <= " << str << std::endl;

    str.push_back('b');
    std::cout << "str <= " << str << std::endl;

    return 0;
}
```

```
str <= a
str <= ab
```
