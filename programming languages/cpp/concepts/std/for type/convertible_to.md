---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::convertible_to<From, To>`** из заголовка `<concepts>` проверяет, можно ли **неявно преобразовать** объект типа `From` в тип `To`.
### Что делает `std::convertible_to<From, To>`?

Он возвращает `true`, если выражение:
```cpp
To obj = std::declval<From>();
```
корректно компилируется (без явного приведения).

✅ Это включает:
- Приведение арифметических типов (`int → double`),
- Upcast по иерархии наследования (`Derived* → Base*`),
- Преобразование с помощью `operator T()` или конструкторов,
- Но **не включает** явные приведения (`static_cast`, `dynamic_cast`).

### Важные моменты

| Выражение | Результат |
|----------|----------|
| `std::convertible_to<int, double>` | ✅ `true` |
| `std::convertible_to<double, int>` | ❌ `false` |
| `std::convertible_to<Derived*, Base*>` | ✅ `true` |
| `std::convertible_to<Base*, Derived*>` | ❌ `false` |
| `std::convertible_to<int&, int>` | ✅ `true` — копирование разрешено |
| `std::convertible_to<void, int>` | ❌ `false` |

> 💡 Концепт учитывает:
- `operator T()`,
- Неявные конструкторы,
- Правила стандартных преобразований C++.

### Лучшая практика

| Совет                                                                 | Почему                           |
| --------------------------------------------------------------------- | -------------------------------- |
| Используйте `convertible_to` вместо ручных `enable_if`                | Читаемее                         |
| Документируйте ожидаемое преобразование                               | Особенно в API                   |
| Избегайте слишком широких ограничений                                 | Может принять нежелательные типы |
| Предпочитайте `same_as` или `derived_from`, если нужно точное условие | Более строго                     |

```cpp
#include <iostream>
#include <concepts>
  
template<typename T>
requires std::convertible_to<T, double>
void test_number(T);

template<typename T>
requires std::convertible_to<T, std::string>
void test_string(T);

int main() {
    test_number(42);
    test_number(2.72f);
    test_string("hi");
    test_string(std::string("hello"));

    return 0;
}

template<typename T>
requires std::convertible_to<T, double>
void test_number(T _input) {
    std::cout
        << "Numberic: "
        << static_cast<double>(_input)
        << std::endl;
}

template<typename T>
requires std::convertible_to<T, std::string>
void test_string(T _input) {
    std::cout
        << "String: "
        << std::string(_input)
        << std::endl;
}
```

```
Numberic: 42
Numberic: 2.72
String: hi
String: hello
```
