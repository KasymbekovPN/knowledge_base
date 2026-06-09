---
tags:
  - programming-language
  - cpp
  - RAII
---
[[RAII|<=]]

В C++ для безопасной и исключение-устойчивой работы с мьютексами используются **RAII-обёртки**, которые автоматически захватывают мьютекс при создании объекта и освобождают — при уничтожении (выходе из области видимости).

> ✅ Это гарантирует, что:
> - Мьютекс будет разблокирован даже при выбросе исключения,
> - Не будет deadlock'ов из-за забытого `unlock()`.

### Особенности lock_guard:
- Только конструктор захватывает мьютекс,
- Нельзя явно разблокировать,
- Нельзя перемещать или копировать,
- Очень эффективна.

```cpp
#include <iostream>
#include <mutex>

std::mutex mtx;
int shared_data;

void safe_function();

int main() {
    safe_function();

    return 0;
}

void safe_function() {
    // capture the mutex
    std::lock_guard<std::mutex> lock(mtx);

    // critical section
    shared_data++;
    std::cout << "shared_data => " << shared_data << std::endl;

} // destructor calling -> mtx.unlock()
```

```
shared_data => 1
```
