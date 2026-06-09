---
tags:
  - programming-language
  - cpp
  - syntax
  - namespace
---
[[__cpp syntax namespaces__|<=]]

Для определения пространства имен применяется ключевое слово _namespace_, за которым идет название имени пространства имен. После имени пространства имен идет блок кода, в который собственно помещаются компоненты пространства имен - функции, классы и т.д.

Внутри пространства имен к его компонентам можно обращаться без имени пространства имен.

```cpp
namespace name {
	// code
}
```

```cpp
#include <iostream>

namespace hello {
    const std::string MESSAGE {"hello"};
    void print(const std::string&);
    void print_default();
}

int main(int argc, char const *argv[]) {
    hello::print("test text");
    hello::print_default();

    return 0;
}

void hello::print(const std::string& text) {
    std::cout << text << std::endl;
}

void hello::print_default() {
    print(MESSAGE);
}
```

```
test text
hello
```

---
[Пространство имен](https://metanit.com/cpp/tutorial/5.16.php)