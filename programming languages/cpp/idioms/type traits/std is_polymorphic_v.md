---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_polymorphic_v<T>` из заголовка `<type_traits>` проверяет, является ли **класс или структура полиморфной** — то есть имеет хотя бы одну **виртуальную функцию**.

### Что значит "полиморфный тип"?

Тип `T` считается полиморфным, если:
- Это класс или структура,
- И он объявляет или наследует **хотя бы одну виртуальную функцию** (включая деструктор).

> 💡 Такие типы:
> - Могут участвовать в `dynamic_cast`,
> - Имеют скрытый указатель `vptr` → размер больше,
> - Позволяют динамическое связывание вызовов.

### Важные моменты

| Выражение                        | Результат                     |
| -------------------------------- | ----------------------------- |
| `std::is_polymorphic_v<Base>`    | ✅ `true`, если есть `virtual` |
| `std::is_polymorphic_v<int>`     | ❌ `false` (не класс)          |
| `std::is_polymorphic_v<const T>` | То же, что и `T`              |
| `std::is_polymorphic_v<T&>`      | ❌ `false` (ссылка)            |

> 📌 Только **классы с виртуальными функциями** считаются полиморфными.

### Лучшая практика

| Совет                                                                  | Почему                                                  |
| ---------------------------------------------------------------------- | ------------------------------------------------------- |
| Делайте деструктор `virtual`, если класс предназначен для наследования | Безопасное освобождение памяти                          |
| Не используйте виртуальные функции без необходимости                   | Добавляют накладные расходы (`vtable`, `vptr`)          |
| Комбинируйте с `if constexpr` для оптимизации                          | Например, использовать `dynamic_cast` только если можно |
| Избегайте множественного наследования с виртуалами без нужды           | Сложность управления объектом                           |

```cpp
#include <iostream>
#include <type_traits>

struct V3 {
    float x, y, z;
};

struct Animal {
    virtual void speak() = 0;
};

struct Dog: Animal {
    void speak() override {
        std::cout << "Woof" << std::endl;
    }
};

template<typename T>
void test(const std::string&&);

int main() {
    test<V3>("V3");
    test<Animal>("Animal");
    test<Dog>("Dog");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    std::cout << "[" << _lbl << "]: ";
    if constexpr (std::is_polymorphic_v<T>) {
        std::cout << "+";
    } else {
        std::cout << "-";
    }
    std::cout << std::endl;
}
```

```
[V3]: -    
[Animal]: +
[Dog]: +
```
