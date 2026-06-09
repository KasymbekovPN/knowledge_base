---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_default_constructible_v<T>` из заголовка `<type_traits>` проверяет, можно ли **создать объект типа `T` по умолчанию**, то есть с помощью:
```cpp
T obj;
T{};
```

### Что значит "default constructible"?

Тип `T` является **default constructible**, если:
- У него есть **конструктор по умолчанию** (явный или автоматически сгенерированный),
- И этот конструктор **не удалён (`= delete`)**,
- И он **доступен** (например, не `private`, если создаётся вне класса).

✅ Это включает:
- Примитивные типы: `int`, `double`
- Классы без пользовательских конструкторов
- Структуры с полями, но без конструкторов
- Типы с `T() = default;`

❌ Не включает:
- Классы с `T() = delete;`
- Классы, где единственный конструктор требует параметров
- Абстрактные классы (если только не абстрактный шаблон, но это редкость)

```cpp
#include <iostream>
#include <type_traits>

struct Point {
    int x, y;
};

class NonDefault {
public:
    explicit NonDefault(int value) {}
};

class Defaultable {
public:
    int x;
    Defaultable() = default;
};

class NoDefault {
public:
    NoDefault() = default;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<int>("int");
    test<double>("double");
    test<Point>("Point");
    test<Defaultable>("Defaultable");
    test<NonDefault>("NonDefault");
    test<NoDefault>("NoDefault");
    test<std::string>("std::string");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_dc = std::is_default_constructible_v<T>;
    std::cout << "[" << _lbl << "]: "
        << std::boolalpha
        << is_dc
        << std::noboolalpha
        << std::endl;
}
```

```
[int]: true
[double]: true
[Point]: true
[Defaultable]: true
[NonDefault]: false
[NoDefault]: true
[std::string]: true
```
