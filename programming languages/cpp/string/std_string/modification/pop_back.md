---
tags:
  - programming-language
  - cpp
  - syntax
  - string
---
[[programming languages/cpp/string/std_string/modification/_|<=]]

Метод `pop_back()` удаляет последний символ (__C++11__)

Метод `pop_back()` **предполагает, что строка не пустая** , и удаляет **последний символ** . Если вызвать его на пустой строке — это **нарушение предусловия** , и результат **не определён** .

```cpp
#include <iostream>
#include <string>

void _test_pop_back(std::string&);

int main() {
    std::string str {"01"};

    const size_t SIZE = str.size();
    for (size_t i {}; i <= SIZE; i++) {
        _test_pop_back(str);
    }

    return 0;
}

void _test_pop_back(std::string& str) {
    if (str.empty()) {
        std::cout << "It's empty" << std::endl;
    } else {
        std::cout << str <<  std::endl;
        str.pop_back();
    }
}
```

```
01
0
It's empty
```
