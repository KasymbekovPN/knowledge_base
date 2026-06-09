---
tags:
  - programming-language
  - cpp
  - syntax
  - template
  - method
---
[[_cpp syntax template|<=]]

Стоит отметить, что начиная со стандарта __C++20__ можно определять параметры, типы которых автоматически выводятся исходя из переданных аргументов. Аналогично можно и выводить тип результата. Для этого применяется ключевое слово _auto_. Равным образом для определения типов параметров и результатов функции можно использовать выражения `auto*`, `auto&` и `const auto&`.

```cpp
#include <iostream>

auto add(const auto& a, const auto& b) {
    return a + b;
}

int main(int argc, char const *argv[]) {
    std::cout << "int\tresult <= " << ::add(1, 3) << std::endl;
    std::cout << "double\tresult <= " << ::add(1.2, 3.4) << std::endl;

    return 0;
}
```

```
int     result <= 4
double  result <= 4.6
```

---
[Шаблоны функций](https://metanit.com/cpp/tutorial/9.2.php)