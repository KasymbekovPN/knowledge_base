---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::movable<T>`** из заголовка `<concepts>` проверяет, что тип `T` можно безопасно **перемещать и присваивать**, что является базовым требованием для работы с STL-контейнерами.

## 📘 Что означает `std::movable<T>`?

Тип `T` удовлетворяет концепту, если он:
1. Является **деструктируемым**: `std::destructible<T>`
2. Можно создать из rvalue: `std::constructible_from<T, T&&>`
3. Можно присвоить из rvalue: `std::assignable_from<T&, T&&>`
4. После перемещения объект остаётся в **влипаемом состоянии** (weakly-regular)

✅ Это **основа move semantics** в C++.

### Важные моменты

| Тип                          | `movable`?         |
| ---------------------------- | ------------------ |
| `int`, `double`              | ✅ Да               |
| `std::string`, `std::vector` | ✅ Да               |
| `std::unique_ptr<T>`         | ✅ Да               |
| `std::mutex`                 | ❌ Нет (`= delete`) |
| `std::thread`                | ✅ Да               |
| `int&`, `const char*`        | ❌ Нет              |
| `std::array<int, 5>`         | ✅ Да               |

> 💡 Все RAII-обёртки (`unique_ptr`, `lock_guard`, `thread`) — **movable**, но не copyable.

### Лучшая практика

| Совет                                                            | Почему                            |
| ---------------------------------------------------------------- | --------------------------------- |
| Всегда объявляйте `operator=` `noexcept`                         | Для эффективности в `std::vector` |
| Если нужен move-only тип — запретите копирование, разрешите move | Как `unique_ptr`                  |
| Используйте `= default` при возможности                          | Компилятор сделает правильно      |
| Проверяйте `movable` в шаблонах                                  | Безопасность                      |

```cpp
#include <iostream>
#include <concepts>
#include <vector>

struct NoMove {
    NoMove(NoMove&&) = delete;
    NoMove& operator=(NoMove&&) = delete;
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

template<std::movable T>
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
