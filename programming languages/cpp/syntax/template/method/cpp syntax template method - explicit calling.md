---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - method
---
[[_cpp syntax template|<=]]

Имеется возможность явно указать компилятору конкретный тип.

```cpp
#include <iostream>

template<typename T> T add(T, T);

int main(int argc, char const *argv[]) {
    std::cout << "implicit\t: " << add(12.3, 45.6) << std::endl;
    std::cout << "explicit\t: " << add<int>(12.3, 45.6) << std::endl;

    return 0;
}

template<typename T> T add(T a, T b) {
    return a + b;
}
```

```
implicit: 57.9
explicit: 57
```

---
[Шаблоны функций](https://metanit.com/cpp/tutorial/9.2.php)