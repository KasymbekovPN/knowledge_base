---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::copyable<T>`** из заголовка `<concepts>` проверяет, что тип можно **копировать и присваивать**, то есть он поддерживает семантику значений (value semantics).

### Что означает `std::copyable<T>`?

Тип `T` удовлетворяет концепту, если он одновременно:
1. `std::movable<T>` — можно перемещать,
2. `std::assignable_from<T&, const T&>` — можно присвоить из константной ссылки.

✅ Это означает, что тип:
- Можно копировать: `T a = b;`
- Можно присваивать: `a = b;`
- Является "обычным" типом, как `int`, `double`, `std::string`.

### Важные моменты

| Тип                                        | `copyable`?                          |
| ------------------------------------------ | ------------------------------------ |
| `int`, `double`                            | ✅ Да                                 |
| `std::string`, `std::vector`, `std::array` | ✅ Да                                 |
| `std::pair<int, double>`                   | ✅ Да                                 |
| `std::unique_ptr<T>`                       | ❌ Нет                                |
| `std::mutex`                               | ❌ Нет                                |
| `std::function<void()>`                    | ❌ Нет (может быть, но не обязателен) |
| `int&`                                     | ❌ Нет — ссылки не копируются         |

> 💡 Ссылки, потоки, мьютексы — **не копируемые**, только перемещаемые.

### Лучшая практика

| Совет                                                              | Почему                           |
| ------------------------------------------------------------------ | -------------------------------- |
| Используйте `= default` для `operator=` и конструктора копирования | Компилятор сделает правильно     |
| Явно запрещайте копирование через `= delete`                       | Чёткое намерение                 |
| Документируйте требования в API                                    | Особенно в библиотечном коде     |
| Предпочитайте `std::movable` для ресурсных типов                   | Например: `unique_ptr`, `thread` |
```cpp
#include <iostream>
#include <concepts>
#include <vector>

struct NoCopy {
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = default;
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

template<std::copyable T>
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
