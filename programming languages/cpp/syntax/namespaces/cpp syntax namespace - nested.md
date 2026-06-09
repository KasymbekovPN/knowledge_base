---
tags:
  - programming-language
  - cpp
  - syntax
  - namespace
---
[[__cpp syntax namespaces__|<=]]

Одно пространство имен может содержать другие пространства. Вне пространства имен надо указывать всю цепь пространств имен.

```cpp
#include <iostream>

namespace console {
    namespace messages {
        const std::string hello {"Hello"};
        const std::string welcome {"Welcome"};
        const std::string goodbye {"Good bye"};
    }
    void print(const std::string&);
    void print_default();
}

int main(int argc, char const *argv[]) {
    console::print_default();
    console::print(console::messages::hello);
    console::print(console::messages::goodbye);

    return 0;
}

void console::print(const std::string& text) {
    std::cout << text << std::endl;
}

void console::print_default() {
    print(messages::welcome);
}
```

```
Welcome
Hello
Good bye
```

---
[Пространство имен](https://metanit.com/cpp/tutorial/5.16.php)