---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::is_final_v<T>` из заголовка `<type_traits>` проверяет, объявлен ли **класс или структура** с модификатором `final`, то есть **не может быть унаследован далее**.

### Что значит `final`?

Ключевое слово `final` в C++ используется для:
- Запрета наследования от класса:
  ```cpp
  class Base final { };
  ```
- Запрета переопределения виртуальной функции:
  ```cpp
  virtual void foo() final;
  ```

⚠️ `std::is_final_v<T>` проверяет **только первый случай** — является ли **класс/структура** помеченной как `final`.

> ❌ Не работает для методов.

### Важные моменты

| Выражение | Результат |
|----------|----------|
| `std::is_final_v<struct S final {}>` | ✅ `true` |
| `std::is_final_v<struct S {}>` | ❌ `false` |
| `std::is_final_v<int>` | ❌ `false` (не класс) |
| `std::is_final_v<const T>` | То же, что и `T` |
| `std::is_final_v<T&>` | ❌ `false` |

> 📌 Только классы и структуры могут быть `final`.  
> Переменные, функции, примитивы — нет.

### Когда использовать `final`?

| Ситуация | Почему |
|---------|--------|
| Хочешь запретить наследование | Например, реализация завершена |
| Реализуешь «запечатанный» API | Чтобы пользователи не расширяли |
| Улучшаешь производительность | Компилятор может убрать виртуальные вызовы |
| Используешь CRTP или паттерны финальных классов | Для безопасности |

### Лучшая практика

| Совет | Почему |
|------|--------|
| Используйте `final` для классов, которые не предназначены для наследования | Ясность намерений |
| Комбинируйте с `override` и `final` для методов | Полный контроль |
| Используйте `if constexpr(std::is_final_v<T>)` в метапрограммировании | Для выбора стратегии |
| Избегайте `final` без причины | Может ограничить расширяемость |

```cpp
#include <iostream>
#include <type_traits>

struct Base {};

struct FinalClass final {};

struct AnotherFinal final {
    int data;
};

template<typename T>
struct Wrapper final {
    T value;
};

template<typename T>
void test(const std::string&&);

int main() {
    test<Base>("Base");
    test<FinalClass>("FinalClass");
    test<AnotherFinal>("AnotherFinal");
    test<Wrapper<int>>("Wrapper<int>");
    test<int>("int");

    return 0;
}

template<typename T>
void test(const std::string&& _lbl) {
    constexpr bool is_final = std::is_final_v<T>;
    std::cout
        << "[" << _lbl << "]: "
        << std::boolalpha
        << is_final
        << std::noboolalpha
        << std::endl;
}
```

```
[Base]: false
[FinalClass]: true
[AnotherFinal]: true
[Wrapper<int>]: true
[int]: false
```
