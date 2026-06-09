---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_empty_v<T>` из заголовка `<type_traits>` проверяет, является ли **класс или структура "пустыми" (empty class)** — то есть **не содержат нестатических данных-членов**.
### Что значит "пустой тип"?

Тип считается пустым (`empty`), если:
- Это класс или структура,
- У него **нет нестатических полей данных**,
- Может иметь функции, виртуальные функции, `static` члены, вложенные типы.

> 💡 Пустые классы используются для метапрограммирования, тегов, EBO (Empty Base Optimization).

### Важные моменты

| Выражение                      | Результат            |
| ------------------------------ | -------------------- |
| `std::is_empty_v<struct {}>`   | ✅ `true`             |
| `std::is_empty_v<int>`         | ❌ `false` (не класс) |
| `std::is_empty_v<std::string>` | ❌ `false`            |
| `std::is_empty_v<const Empty>` | ✅ `true`             |
| `std::is_empty_v<Empty&>`      | ❌ `false` (ссылка)   |
### Лучшая практика

| Совет                                                      | Почему                                 |
| ---------------------------------------------------------- | -------------------------------------- |
| Используйте пустые классы как **теги** или **метки типа**  | Например: `struct fast_policy_tag {};` |
| Комбинируйте с `if constexpr` и `is_empty_v`               | Для выбора стратегии хранения          |
| Используйте наследование (`: Tag`) вместо поля (`Tag tag`) | Чтобы включить EBO                     |
| Не путайте "пустой" и "тривиальный"                        | Это разные понятия                     |

```cpp
#include <iostream>
#include <type_traits>

struct Empty {};
  
class EmptyClass {
    void method() {}
    static int value;
};

struct Base : Empty {};

struct NonEmpty {
    int x;
};

struct HasStatic {
    static const int size = 42;
    void foo() {}
};

template<typename T>
void test_print(const std::string&&);

int main() {
    test_print<Empty>("Empty");
    test_print<EmptyClass>("EmptyClass");
    test_print<Base>("Base");
    test_print<NonEmpty>("NonEmpty");
    test_print<HasStatic>("HasStatic");
    test_print<int>("int");
    test_print<void>("void");

    return 0;
}

template<typename T>
void test_print(const std::string&& _lbl) {
    std::cout << _lbl;
    if constexpr (std::is_empty_v<T>) {
        std::cout << " - empty";
    } else {
        std::cout << " - no empty";
    }
    std::cout << ::std::endl;
}
```

```
Empty - empty
EmptyClass - empty
Base - empty
NonEmpty - no empty
HasStatic - empty
int - no empty
void - no empty
```
