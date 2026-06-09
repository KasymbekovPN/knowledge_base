---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/size/_|<=]]

Метод `size()` возвращает длину строки.

```cpp
#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"Example from literal"};
    std::cout << "size <= " << sv.size() << std::endl;

    return 0;
}
```

```
size <= 20
```
