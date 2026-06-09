---
tags:
  - programming-language
  - cpp
  - syntax
  - enum
  - constants
---
[[__cpp syntax oop enums__|<==]]

При обращении к константам перечисления по умолчанию необходимо указывать название перечисления, например, _Day::Monday_. Но начиная со стандарта __C++20__ мы можем подключить константы перечисления в текущий контекст с помощью оператора _using_.

```cpp
#include <iostream>

enum class Day {
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday,
	Sunday
};

using enum Day;

int main(int argc, char const *argv[]) {
    Day today {Friday};
    std::cout << static_cast<int>(today) << std::endl;
    return 0;
}
```

```
4
```

---
[Перечисления](https://metanit.com/cpp/tutorial/5.9.php)