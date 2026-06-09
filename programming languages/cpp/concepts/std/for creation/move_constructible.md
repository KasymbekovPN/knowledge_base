---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::move_constructible<T>`** из заголовка `<concepts>` проверяет, можно ли создать объект типа `T`, используя **перемещение (move)** из rvalue-выражения того же типа.

### Что означает `std::move_constructible<T>`?

Тип `T` удовлетворяет концепту, если выражение:
```cpp
T(std::declval<T&&>())
```
корректно компилируется.

✅ Это означает:
- У `T` есть **конструктор перемещения** (`T(T&&)`),
- Или он является типом, который можно скопировать/переместить по умолчанию (например, фундаментальный тип).

### Важные моменты

| Тип                          | `move_constructible`?                  |
| ---------------------------- | -------------------------------------- |
| `int`, `double`              | ✅ Да                                   |
| `std::string`, `std::vector` | ✅ Да                                   |
| `std::unique_ptr<T>`         | ✅ Да                                   |
| `std::mutex`                 | ❌ Нет (не перемещается, не копируется) |
| `int&`, `void* const`        | ❌ Нет                                  |
| Массив `int[5]`              | ✅ Да (aggregate)                       |
| `std::array<int, 5>`         | ✅ Да                                   |

> 💡 Все стандартные RAII-обёртки (`unique_ptr`, `lock_guard`, `thread`) — перемещаемы, но не копируемы.

### Лучшая практика

| Совет                                                          | Почему                                                 |
| -------------------------------------------------------------- | ------------------------------------------------------ |
| Всегда объявляйте `T(T&&)` `noexcept`                          | Для эффективности в `std::vector` и других контейнерах |
| Если нужен move-only тип — удалите копирование, разрешите move | Как `unique_ptr`                                       |
| Используйте `= default` при возможности                        | Компилятор сделает правильно                           |
| Проверяйте `move_constructible` в шаблонах                     | Безопасность                                           |

```cpp
#include <iostream>
#include <concepts>
#include <vector>

struct NoMove {
    NoMove(NoMove&&) = delete;
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

template<std::move_constructible T>
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
    test<std::unique_ptr<int>>();
    test<Point>();
    // test<NoMove>(); // Error

    return 0;
}
```

```
int
double
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
class std::vector<int,class std::allocator<int> >
class std::unique_ptr<int,struct std::default_delete<int> >
class Point
```
