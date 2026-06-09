---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::decay_t<T>`** из заголовка `<type_traits>` применяет стандартные преобразования типов, которые происходят при передаче аргумента по значению в шаблонную функцию.

### Что делает `std::decay_t<T>`?

Он выполняет три преобразования (в порядке):
1. **Удаляет ссылки**: `T&`, `T&&` → `T`
2. **Превращает массивы в указатели**: `T[N]` → `T*`, `T[]` → `T*`
3. **Превращает функции в указатели на функции**: `void(int)` → `void(*)(int)`
4. Удаляет `const`, `volatile`, и другие квалификаторы (но не с данными!)

✅ Это **точно соответствует поведению вывода типа `T` в `template<typename T> void func(T)`**.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int&, int>();
    test<int&&, int>();
    test<const int&, int>();
    test<int[5], int*>();
    test<char[], char*>();
    test<void(int), void(*)(int)>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::decay_t<T>, R>
        << std::endl;
}
```

```
true
true
true
true
true
true
```
