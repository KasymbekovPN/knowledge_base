---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_abstract_v<T>` из заголовка `<type_traits>` проверяет, является ли **класс или структура абстрактным** — то есть имеет хотя бы одну **чисто виртуальную функцию**, или унаследован от абстрактного класса.
### Что значит "абстрактный тип"?

Тип считается абстрактным, если:
- Он содержит **хотя бы одну чисто виртуальную функцию** (`virtual void func() = 0`),
- Или наследует такую функцию и не переопределяет её.

### Важные моменты

| Выражение                                                | Результат                                |
| -------------------------------------------------------- | ---------------------------------------- |
| `std::is_abstract_v<struct S { virtual void f() = 0; }>` | ✅ `true`                                 |
| `std::is_abstract_v<Dog>`                                | ❌ `false` (все `=0` функции реализованы) |
| `std::is_abstract_v<int>`                                | ❌ `false`                                |
| `std::is_abstract_v<const T>`                            | То же, что и `T`                         |

> 📌 Только **классы/структуры с нереализованными чистыми виртуальными функциями** считаются абстрактными.

### Лучшая практика

| Совет                                                                           | Почему                      |
| ------------------------------------------------------------------------------- | --------------------------- |
| Используйте абстрактные классы для определения интерфейсов                      | Чистый API                  |
| Делайте деструктор `virtual` и часто `= default`, если есть виртуальные функции | Безопасное удаление         |
| Не пытайтесь создавать объекты абстрактных классов                              | Ошибка компиляции           |
| Комбинируйте с `std::unique_ptr<Base>`                                          | Управление жизненным циклом |

```cpp
#include <iostream>
#include <type_traits>

struct Interface {
    virtual void do_sth() = 0;
};

struct AbstractBase {
    virtual ~AbstractBase() = 0;
};

struct Animal {
    virtual void speak() = 0;
    virtual ~Animal() = default;
};

struct Dog: Animal {
    void speak() override {}
};

struct Empty {};

template<typename T>
void test(const std::string&&);

int main() {
    test<Interface>("Interface");
    test<AbstractBase>("AbstractBase");
    test<Animal>("Animal");
    test<Dog>("Dog");
    test<Empty>("Empty");
    test<int>("int");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    std::cout << "[" << _lbl << "]: ";
    constexpr bool is_abstract = std::is_abstract_v<T>;
    std::cout
        << std::boolalpha
        << is_abstract
        << std::noboolalpha
        << std::endl;
}
```

```
[Interface]: true
[AbstractBase]: true
[Animal]: true
[Dog]: false
[Empty]: false
[int]: false
```
