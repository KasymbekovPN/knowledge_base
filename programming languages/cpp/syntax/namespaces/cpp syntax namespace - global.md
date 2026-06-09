---
tags:
  - programming-language
  - cpp
  - syntax
  - namespace
---
[[__cpp syntax namespaces__|<=]]

Если __пространство имен__ не указано, то по умолчанию применяется глобальное пространство имен. применяется по умолчанию, если пространство имен не было определено. Все имена в глобальном пространстве имен такие же, как вы их объявляете, без прикрепления имени пространства имен.

В примере определены функции _print_ и _main_ и константа _MESSAGE_ и не используется никакого пространства имен. Поэтому фактически функции _print_ и _main_ и константа _MESSAGE_ определены в глобальном пространстве имен. В принципе для обращения к ним также можно использовать оператор `::`, только без названия пространства имен, хотя это и избыточно.

```cpp
#include <iostream>

const std::string MESSAGE {"Hello, world !!!"};

void print(const std::string&);

int main(int argc, char const *argv[]) {
    print(MESSAGE);
    ::print(MESSAGE);
    return 0;
}

void print(const std::string& text) {
    std::cout << text << std::endl;
}
```

```
Hello, world !!!
Hello, world !!!
```

__Важно__: _main_ должна быть определена в глобальном пространстве имён.

---
[Пространство имен](https://metanit.com/cpp/tutorial/5.16.php)