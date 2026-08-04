---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]


`[[noreturn]]` — атрибут, которым помечают функцию, гарантируя компилятору, что она никогда не возвращает управление вызывающему коду (то есть либо всегда бросает исключение, либо вызывает `std::abort`/`std::exit`, либо уходит в бесконечный цикл, либо передаёт управление через `longjmp`); это позволяет компилятору не генерировать код после вызова такой функции, подавлять ложные предупреждения вида "функция не возвращает значение" или "код недостижим", и иногда точнее оптимизировать поток управления. Пример: `[[noreturn]] void fail(const char* msg) { throw std::runtime_error(msg); }`. Важно — это обещание компилятору, а не проверка: если функция всё же вернётся, поведение неопределено (UB), поэтому атрибут нужно ставить только тогда, когда это действительно гарантировано.

```cpp
#include <iostream>  
#include <format>  
#include <stdexcept>  
  
[[noreturn]] static void fail(const char* msg) {  
    throw std::runtime_error(msg);  
}  
  
static int process(const int x) {  
    if (x < 0) fail("negative value");  
    return 2 * x;  
}  
  
int main() {  
    try {  
        std::cout << process(-1) << std::endl;  
    } catch (const std::exception& e) {  
        std::cerr << std::format("caught: {}\n", e.what());  
    }}
```

Если убрать `throw` и оставить, например, просто `std::cerr << msg;` без выхода — функция вернётся, атрибут окажется нарушен, и это UB (компилятор мог уже не сгенерировать код после её вызова).
