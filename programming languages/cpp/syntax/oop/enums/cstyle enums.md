---
tags:
  - programming-language
  - cpp
  - syntax
  - enum
---
[[__cpp syntax oop enums__|<==]]

Стоит отметить, что раньше в __С++__ использовалась другая форма перечислений, которые пришли из языка __С__ и определяются без ключевого слова _class_.

Такие перечисления еще называют `unscoped` (то есть не ограниченные ни какой областью видимостью). Естественно такие перечисления можно встретить в старых программах. Однако в виду того, что они потенциально могут привести к большему количеству ошибок, то в настоящее время такая форма все меньше и меньше используется.

```cpp
#include <iostream>

enum Day {Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};

int main(int argc, char const *argv[]) {
    Day today = Tuesday;
    std::cout << today << std::endl;

    return 0;
}
```

```
1
```

---
[Перечисления](https://metanit.com/cpp/tutorial/5.9.php)