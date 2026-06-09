---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/clear/_|<=]]

Очищает строку

```cpp
#include <iostream>
#include <string>

int main() {
    std::string str {"0123456789"};
    std::cout << "size: " << str.size() << std::endl;

    str.clear();
    std::cout << "size: " << str.size() << std::endl;

    return 0;
}
```

```
size: 10
size: 0
```
