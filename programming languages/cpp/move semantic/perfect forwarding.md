---
tags:
  - programming-language
  - cpp
  - move-semantic
---
[[programming languages/cpp/move semantic/_|<=]]

**Perfect Forwarding** — это механизм в __C++__, который позволяет **передавать параметры функции дальше, сохранив их категорию значения (lvalue/rvalue)** и тип. Это особенно важно при работе с шаблонами.

```cpp
#include <iostream>
#include <utility>

using std::cout;
using std::endl;

void print(int& x) {
    cout << "lvalue: " << x << endl;
}

void print(int&& x) {
    cout << "rvalue: " << x << endl;
}

template<typename T>
void wrapper(T&& x) {
    print(std::forward<T>(x)); // perfect foirwarding
}

int main() {
    int a {42};
    wrapper(a);
    wrapper(12);
    wrapper(std::move(a));

    return 0;
}
```

```
lvalue: 42
rvalue: 12
rvalue: 42
```

### Как работает `std::forward`

Функция `std::forward<T>()` не делает ничего сама по себе, но она:
- Возвращает `T&`, если аргумент был **lvalue**,
- Возвращает `T&&`, если аргумент был **rvalue**.

Таким образом, она сохраняет исходную категорию значения.

### Когда использовать `std::forward`

Используй `std::forward<T>`:
- При написании **универсальных обёрток**,
- При реализации **perfect constructors** или **фабричных функций**,
- При работе с **шаблонными лямбдами**,
- Чтобы поддерживать **любые типы аргументов** без потери информации о категории значения.

### Пример: фабричная функция с perfect forwarding

```cpp
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
```

Эта реализация:
- Поддерживает любое количество аргументов,
- Сохраняет категорию значения каждого аргумента,
- Работает с любыми типами, поддерживая move семантику.

### Отличие `std::forward` от `std::move`

| Метод                | Что делает                                          | Когда использовать                                           |
| -------------------- | --------------------------------------------------- | ------------------------------------------------------------ |
| `std::move(x)`       | Приводит к `T&&` → всегда rvalue                    | Когда хочешь переместить объект                              |
| `std::forward<T>(x)` | Возвращает `T&` или `T&&` в зависимости от типа `T` | Когда нужно передать значение дальше, сохранив его категорию |

### Perfect Forwarding и ссылочная склейка (reference collapsing)

Важно понимать, как работают шаблоны с `T&&`.

| Выражение | Тип `T` | Результат `T&&` |
|-----------|----------|------------------|
| `int x; wrapper(x);` | `int&` | `int& && → int&` |
| `wrapper(5);` | `int` | `int&&` |
| `wrapper(std::move(x));` | `int` | `int&&` |

Это правило называется **reference collapsing**:
- `T& & → T&`
- `T& && → T&`
- `T&& & → T&`
- `T&& && → T&&`

### Полезные утилиты для проверки

```cpp
#include <type_traits>

template<typename T>
void print_type() {
    if (std::is_lvalue_reference_v<T>)
        std::cout << "Lvalue\n";
    else if (std::is_rvalue_reference_v<T>)
        std::cout << "Rvalue\n";
    else
        std::cout << "Other\n";
}
```
