---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[nodiscard]]` — атрибут на функции, классе или enum, требующий не игнорировать возвращаемое значение: если результат вызова отброшен (не присвоен переменной, не использован в выражении), компилятор выдаёт предупреждение. С C++20 можно добавить пояснение — `[[nodiscard("причина")]]`. Если атрибут стоит на классе, он автоматически распространяется на все функции, возвращающие этот тип.

```cpp
#include <iostream>  
  
[[nodiscard("int-compute")]]  
static int compute() { return 42; }  
  
namespace {  
    struct [[nodiscard("ErrorCode")]] ErrorCode { int value; };  
}  
  
static ErrorCode get_error_code_v0() { return ErrorCode(); }  
  
[[nodiscard("get_error_code_v1")]]  
static ErrorCode get_error_code_v1() { return ErrorCode(); }  
  
int main() {  
    compute();  
    get_error_code_v0();  
    get_error_code_v1();  
    std::cout << compute() << std::endl;  
  
    return 0;  
}
```

Типичный кейс — предотвратить баги вроде игнорирования кода ошибки или результата `std::async`, где отбрасывание значения обычно означает логическую ошибку в коде.
