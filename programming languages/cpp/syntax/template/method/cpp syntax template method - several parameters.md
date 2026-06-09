---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - method
---
[[_cpp syntax template|<=]]

__C++__ предоставляет использовать несколько параметров для создания шаблонных методов.

```cpp
#include <iostream>

template <typename T, typename K> void print(T, T, K, unsigned);

int main(int argc, char const *argv[]) {
    ::print("hello", "world", -123, 123);
    ::print(12.3, 34.5, std::string{"hello"}, 999);

    return 0;
}

template <typename T, typename K>
void print(T t0, T t1, K k, unsigned u) {
    std::cout
        << "{t0: " << t0
        << ", t1: " << t1
        << ", k: " << k
        << ", u: " << u
        << "}" << std::endl;
}
```

```
{t0: hello, t1: world, k: -123, u: 123}
{t0: 12.3, t1: 34.5, k: hello, u: 999}
```

---
[Шаблоны функций](https://metanit.com/cpp/tutorial/9.2.php)