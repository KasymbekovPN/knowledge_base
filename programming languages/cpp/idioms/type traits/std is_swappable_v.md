---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_swappable_v<T>` из заголовка `<type_traits>` проверяет, можно ли **обменять два объекта типа `T`** с помощью неспециализированного `swap`.

### Что значит "swappable"?

Объекты типа `T` являются **swappable**, если выражение:
```cpp
using std::swap;
swap(t1, t2);
```
корректно компилируется.

✅ Это возможно, если:
- Для `T` определён `swap(T&, T&)` (в той же области видимости),
- Или `T` имеет член-функцию `.swap()` и ADL находит `swap`,
- Или специализация `std::swap` доступна,
- Или тип тривиально копируемый (`trivially_copyable`) — тогда `std::swap` использует `memcpy`.

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

#define TEST(T) \
    std::cout \
        << #T ": " \
        << std::is_swappable_v<T> << "\n"

struct Point {
    int x, y;
};

class NonSwappable {
private:
    ~NonSwappable()  = default;
};

struct Widget {
    int id;
    std::string name;
};

void swap(Widget& _a, Widget& _b) noexcept {
    using std::swap;
    swap(_a.id, _b.id);
    swap(_a.name, _b.name);
}

int main() {
    std::cout << std::boolalpha;

    TEST(int);
    TEST(double);
    TEST(Point);
    TEST(NonSwappable);
    TEST(std::string);
    TEST(std::vector<int>);
    TEST(Widget);

    return 0;
}
```

```
int: true
double: true
Point: true
NonSwappable: false
std::string: true
std::vector<int>: true
Widget: true
```
