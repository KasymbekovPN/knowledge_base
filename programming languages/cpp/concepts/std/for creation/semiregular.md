---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::semiregular<T>`** из заголовка `<concepts>` описывает тип, который ведёт себя как **обычный тип со значением**, но не обязательно поддерживает сравнение на равенство.

### Что означает `std::semiregular<T>`?

Тип `T` удовлетворяет концепту, если он одновременно:
1. `std::copyable<T>` — можно копировать и присваивать,
2. `std::default_initializable<T>` — можно создать по умолчанию (`T{}`).

✅ Это базовый набор операций для "хорошего" типа-значения:  
> Можно копировать, перемещать, присваивать, создавать по умолчанию.

### Важные моменты

| Тип                          | `semiregular`?        |
| ---------------------------- | --------------------- |
| `int`, `double`              | ✅ Да                  |
| `std::string`, `std::vector` | ✅ Да                  |
| `std::array<int, 5>`         | ✅ Да                  |
| `std::pair<int, double>`     | ✅ Да                  |
| `std::optional<int>`         | ✅ Да                  |
| `std::unique_ptr<T>`         | ❌ Нет (не копируется) |
| `std::mutex`                 | ❌ Нет                 |
| `int&`                       | ❌ Нет                 |

> 💡 Все агрегаты и стандартные RAII-обёртки (если копируются) — подходят.

### Лучшая практика

| Совет                                               | Почему                    |
| --------------------------------------------------- | ------------------------- |
| Делайте свои типы `semiregular`, если это уместно   | Поддержка STL, алгоритмов |
| Используйте `= default` для трёх/пяти правил        | Чёткое поведение          |
| Документируйте, где требуется value semantics       | Особенно в API            |
| Предпочитайте `regular` при необходимости сравнения | Полный набор операций     |

```cpp
#include <iostream>
#include <concepts>
#include <vector>

class NoDefault {
private:
    std::string data;
public:
    NoDefault(std::string _data):
        data{_data} {}
};

class NoCopy {
public:
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

class Point {
private:
    int x, y;
public:
    Point() = default;
    Point(int _x, int _y):
        x{_x},
        y{_y} {}
    Point(const Point&) = default;
    Point& operator=(const Point&) = default;
};

template<std::semiregular T>
void test() {
    T a{};
    T b = a;
    T c;
    c = a;
    std::cout
        << typeid(a).name()
        << std::endl;
}

int main() {
    test<int>();
    test<std::string>();
    test<std::vector<int>>();
    test<Point>();
    // test<NoDefault>(); // Error
    // test<NoCopy>(); // Error

    return 0;
}
```

```
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
class std::vector<int,class std::allocator<int> >
class Point
```
