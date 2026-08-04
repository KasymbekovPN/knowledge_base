---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]


`[[deprecated]]` — атрибут, помечающий функцию, класс, переменную, typedef и т.п. как устаревшие: компилятор выдаёт предупреждение при каждом использовании помеченной сущности, не запрещая компиляцию. Есть форма с сообщением — `[[deprecated("используйте newFunc() вместо этого")]]`, — которое компилятор включает в текст варнинга.

```cpp
#include <iostream>  
  
[[deprecated("use new_func instead")]]  
static void old_func() { std::cout << "old_func" << std::endl; }  
  
static void new_func() { std::cout << "new_func" << std::endl; }  
  
int main() {  
    old_func();  
    new_func();  
  
    return 0;  
}
```

Полезен при рефакторинге API — даёт коллегам (и себе) заметить использование устаревшего кода ещё до того, как его удалят, но не ломает сборку сразу (если только не включён `-Werror`).
