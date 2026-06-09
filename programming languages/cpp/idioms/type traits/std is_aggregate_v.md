---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_aggregate_v<T>` из заголовка `<type_traits>` проверяет, является ли тип **агрегатом (aggregate)**.

### Что такое агрегат?

**Агрегат (aggregate)** — это тип, который можно инициализировать с помощью **списков инициализации `{}`**, без определения конструктора.

К агрегатам относятся:
- Массивы,
- Классы (включая структуры), которые:
  - Не имеют пользовательских конструкторов,
  - Не имеют `private` или `protected` нестатических членов (если только не из одного класса),
  - Не имеют виртуальных функций,
  - Не имеют виртуального наследования,
  - Не имеют стандартных значений полей (`= default` внутри объявления).

> ✅ Начиная с C++20, некоторые ограничения ослаблены (например, разрешены `= default`, но не `= value`).

```cpp
#include <iostream>
#include <type_traits>

struct Point {
    int x, y;
};

class Vec3 {
public:
    float x, y, z;
};

class NonAggregate {
public:
    int value;
    NonAggregate(int v): value{v} {}
};

struct Data {
    int a;
    int b = 0;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<Point>("Point");
    test<Vec3>("Vec3");
    test<NonAggregate>("NonAggregate");
    test<int[5]>("int[5]");
    test<Data>("Data");
    test<int>("int");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_aggr = std::is_aggregate_v<T>;
    std::cout << "[" << _lbl << "]: ";
    std::cout
        << std::boolalpha
        << is_aggr
        << std::noboolalpha
        << std::endl;
}
```

```
[Point]: true
[Vec3]: true
[NonAggregate]: false
[int[5]]: true
[Data]: true
[int]: false
```
