---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::assignable_from<T, U>`** из заголовка `<concepts>` проверяет, можно ли присвоить значение типа `U` переменной типа `T`.

## 📘 Что означает `std::assignable_from<T, U>`?

Он возвращает `true`, если выражение:
```cpp
std::declval<T>() = std::declval<U>()
```
корректно компилируется.

✅ Это означает:
- `T` должен быть **lvalue-выражением**, т.е. допускать присваивание,
- У `T` должен быть **оператор присваивания**, принимающий `U`,
- Присваивание должно быть **допустимым** (не `= delete`, не защищён).

### Важные моменты

| Выражение                                | Результат                                              |
| ---------------------------------------- | ------------------------------------------------------ |
| `std::assignable_from<int&, int>`        | ✅ `true`                                               |
| `std::assignable_from<int&, double>`     | ✅ `true` (преобразование разрешено)                    |
| `std::assignable_from<int&, const int&>` | ✅ `true`                                               |
| `std::assignable_from<const int&, int>`  | ❌ `false`                                              |
| `std::assignable_from<int, int>`         | ❌ `false` — `int` — rvalue, не может быть слева от `=` |

> 💡 `T` должен быть таким, чтобы `T()` дал **lvalue** (например, `T&`, `MyClass&`).

### Лучшая практика

| Совет                                                            | Почему                                                   |
| ---------------------------------------------------------------- | -------------------------------------------------------- |
| Используйте `assignable_from` в шаблонах, где нужно присваивание | Безопасность                                             |
| Документируйте требование к типу                                 | Особенно в библиотечном коде                             |
| Предпочитайте его вместо ручных `is_assignable_v`                | Читаемее                                                 |
| Не путайте с `convertible_to`                                    | То первое — про присваивание, второе — про инициализацию |


```cpp
#include <iostream>
#include <concepts>

template<typename T, typename U>
requires std::assignable_from<T&, const U&>
void test(T&, const U&);

int main() {
    int x{0};
    double d{3.14};
    test(x, d);
    test(d, 5);

    const int y {y};
    // test(y, 5); // Error

    return 0;
}

template<typename T, typename U>
requires std::assignable_from<T, U>
void test(T& _target, const U& _input) {
    _target = _input;
    std::cout << _input << " :: " << _target << std::endl;
}
```

```
Input: 3.14
Input: 2.718
Input: 1.414
```
