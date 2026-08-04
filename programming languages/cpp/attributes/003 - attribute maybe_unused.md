---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[maybe_unused]]` — атрибут, подавляющий предупреждение компилятора о неиспользуемой сущности (переменной, параметре функции, поле структуры, typedef и т.п.). Полезен, когда переменная объявлена намеренно, но не используется — например, в условной компиляции, для документирования параметра, или для RAII-объекта, значение которого не нужно.

```cpp
#include <iostream>  
#include <format>  
#include <mutex>  
  
static void process([[maybe_unused]] const int logLevel) {  
#ifndef NDEBUG  
    std::cout << std::format("debug level: {}\n", logLevel);  
#endif  
}  
  
static int value{};  
static std::mutex mtx;  
  
int main() {  
    {        
	    [[maybe_unused]] std::lock_guard<std::mutex> lock(mtx);  
    }    
    process(1);  
  
    return 0;  
}
```

До появления `[[maybe_unused]]` (C++17) для этого использовали трюки вроде `(void)var;` или компилятор-специфичные `__attribute__((unused))`.
