---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::copy_constructible<T>`** из заголовка `<concepts>` проверяет, можно ли создать объект типа `T`, используя **конструктор копирования** из lvalue-выражения того же типа.

### Что означает `std::copy_constructible<T>`?

Тип `T` удовлетворяет концепту, если выражение:
```cpp
T(std::declval<const T&>())
```
корректно компилируется.

✅ Это означает:
- У `T` есть **доступный конструктор копирования** (`T(const T&)`),
- Или он является типом, который можно копировать по умолчанию (например, фундаментальный тип),
- Конструктор не `= delete`,
- Не `private`.

### Важные моменты

| Тип                          | `copy_constructible`?        |
| ---------------------------- | ---------------------------- |
| `int`, `double`              | ✅ Да                         |
| `std::string`, `std::vector` | ✅ Да                         |
| `std::array<int, 5>`         | ✅ Да                         |
| `int*`, `void*`              | ✅ Да                         |
| `std::unique_ptr<T>`         | ❌ Нет                        |
| `std::mutex`                 | ❌ Нет                        |
| `const T`                    | ✅ Да (если `T` copyable)     |
| `volatile T`                 | ✅ Да                         |
| `int&`                       | ❌ Нет — ссылки не копируются |

> 💡 Ссылки и move-only типы не являются `copy_constructible`.

### Лучшая практика

| Совет                                                                      | Почему                       |
| -------------------------------------------------------------------------- | ---------------------------- |
| Явно указывайте `= default` или `= delete` для `operator=` и конструкторов | Чёткое намерение             |
| Используйте `std::copy_constructible` в шаблонах, где нужно копирование    | Безопасность                 |
| Предпочитайте `std::movable` для move-only типов                           | Например: `unique_ptr`       |
| Документируйте требования                                                  | Особенно в библиотечном коде |
```cpp
#include <iostream>
#include <concepts>
#include <vector>

struct NoCopy {
    NoCopy(const NoCopy&) = delete;
};

class Point {
private:
    int x, y;

public:
    Point(const Point& _other) = default;
    Point(Point&& _other): x{_other.x}, y{_other.y} {
        _other.x = _other.y = 0;
    }
    Point& operator=(const Point&) = default;
    Point& operator=(Point&&) = default;
};

template<std::copy_constructible T>
void test() {
    std::cout
        << typeid(T).name()
        << std::endl;
}

int main() {
    test<int>();
    test<double>();
    test<std::string>();
    test<std::vector<int>>();
    // test<std::unique_ptr<int>>(); // Error
    test<Point>();
    // test<NoCopy>(); // Error

    return 0;
}
```

```
int
double
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
class std::vector<int,class std::allocator<int> >
class Point
```
