---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/attributes/_|<=]]

`__attribute__((format(archetype, string-index, first-to-check)))` — говорит компилятору, что функция принимает printf/scanf-подобную форматную строку и variadic-аргументы, чтобы он мог статически проверять соответствие формата и типов аргументов (то, что обычно работает для `printf` "из коробки" — распространяется на собственные обёртки). 

Для метода класса нужно учитывать неявный this.

Основные archetype: `printf`, `scanf`, `strftime`, `gnu_printf`/`gnu_scanf` (GNU-версия формата с расширениями, отличающимися от чистого ISO C).

Практическая польза: без этого атрибута компилятор проверяет формат только для настоящего `printf`/`fprintf`/`sprintf` и т.п. из `<cstdio>` — любая своя обёртка-логгер теряет эту проверку и типичные баги (`%d` для `std::string`, лишний/недостающий аргумент) обнаруживаются только в рантайме или через UB. С атрибутом `-Wformat` (обычно включён в `-Wall`) начинает работать и для обёрток. В C++ стоит помнить, что для типобезопасного форматирования часто предпочтительнее `std::format`/`fmt::format` — они ловят ошибки на этапе компиляции без атрибутов и без variadic C-API вообще, `format`-атрибут больше актуален для legacy C-style API или oбязательной совместимости с `printf`-семейством.

```cpp
#include <cstdio>  
#include <cstdarg>  
  
namespace {  
    // archetype: printf  
    // string-index: 1 (формат — первый аргумент, если это функция-член, счёт с учётом this — со второго)    // first-to-check: 2 (variadic-аргументы начинаются со второго параметра)    __attribute__((format(printf, 1, 2)))  
    void logMessage(const char* fmt, ...) {  
        va_list args;  
        va_start(args, fmt);  
        vprintf(fmt, args);  
        va_end(args);  
    }  

    class Logger {  
    public:  
        __attribute__((format(printf, 2, 3)))  
        void log(const char* fmt, ...) const {  
            va_list args;  
            va_start(args, fmt);  
            vprintf(fmt, args);  
            va_end(args);  
        }    
    };
}  
  
int main() {  
    logMessage("value: %d\n", 42);  
    logMessage("value: %d\n", "hello");  
    logMessage("value: %d\n");  
  
    Logger logger;  
    logger.log("value: %d\n", 43);  
}
```
