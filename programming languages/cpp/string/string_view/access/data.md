---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/string_view/access/_|<=]]

Метод `data()` возвращает указатель на внутренние данные (`const char*`)

```cpp
#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view sv {"Example from literal"};
    std::cout << "data result <= " << sv.data() << std::endl;

    return 0;
}
```

```
data result <= Example from literal
```
