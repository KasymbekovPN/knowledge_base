---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`[[likely]]`/`[[unlikely]]` (C++20) — атрибуты на statement (обычно `if`/`else` ветку или `case` в `switch`), подсказывающие компилятору, какой путь выполнения более вероятен, чтобы он мог соответствующим образом расположить код (более вероятную ветку — по прямому пути, менее вероятную — "в сторону") и настроить branch prediction hints, снижая число промахов предсказания переходов на "горячем" пути.

```cpp
  
#include <iostream>  
#include <ostream>  
  
namespace {  
    int handleError() { return -1; }  
  
    int process(const int x) {  
        if (x > 0) [[likely]] {  
            return 2 * x;  
        } else [[unlikely]] {  
            return handleError();  
        }    
    }  
    
    enum class Status { OK, ERROR };  
  
    int doWork(const Status status) {  
        switch (status) {  
            case Status::OK: [[likely]]  
                return process(42);  
            case Status::ERROR: [[unlikely]]  
                return handleError();  
        }    
    }  
}  
  
int main() {  
    std::cout << doWork(Status::OK) << std::endl;  
  
    return 0;  
}
```

Важно: это подсказка компилятору, а не runtime-профилирование (в отличие от PGO). Ошибочная подсказка не UB, но может ухудшить производительность вместо улучшения. Эффект стоит проверять на реальном коде через Compiler Explorer или бенчмарк — на современных компиляторах и CPU с хорошим branch predictor'ом выигрыш часто минимален, атрибут больше полезен для управления layout'ом кода (например, чтобы "холодный" код обработки ошибок не засорял icache горячего пути).
