---
tags:
  - programming-language
  - cpp
  - syntax
  - data-types
  - static
  - boolean
---
[[__static data types index__|<=]]

[Типы данных|metanit.com](https://metanit.com/cpp/tutorial/2.3.php)

Логический тип _bool_ может хранить одно из двух значений: __true__ и __false__ . 
```cpp
#include <iostream>

int main(int argc, char const *argv[]) {
    bool is_alive {true};
    bool is_dead {false};
    bool is_default_value {};

    std::cout << "is alive <= " << is_alive << "\n";
    std::cout << "is dead <= " << is_dead << "\n";
    std::cout << "is default value <= " << is_default_value << "\n";

    return 0;
}
```

```
is alive <= 1
is dead <= 1
is default value <= 0
```

При выводе значения типа __bool__ преобразуются в _1_, если __true__ и _0_, если __false__. Как правило, данный тип применяется преимущество в условных выражениях, которые будут далее рассмотрены.

Значение по умолчанию для переменных этого типа - __false__.