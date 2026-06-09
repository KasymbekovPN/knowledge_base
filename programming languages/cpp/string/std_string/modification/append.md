---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Добавление строки в конец

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"aaa"};
    str += "bbb";
    str.append("ccc");

    std::cout << "str <= " << str << std::endl;

    return 0;
}
```

```
str <= aaabbbccc
```
