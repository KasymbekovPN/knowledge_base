---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::default_initializable<T>`** из заголовка `<concepts>` проверяет, можно ли создать объект типа `T` с помощью **инициализации по умолчанию**, то есть выражение `T{}` или `T()` корректно.
### Что означает `std::default_initializable<T>`?

Тип `T` удовлетворяет концепту, если:
- Можно написать `T{}` (или `T()`),
- У него есть **доступный конструктор по умолчанию**,
- Или он является **агрегатом**, и его элементы могут быть списочно инициализированы.

✅ Это включает:
- Типы с `T() = default`,
- Классы без пользовательских конструкторов,
- Агрегаты (`struct`, `class`) с полями, допускающими значение по умолчанию.

❌ Не включает:
- Типы с удалённым/приватным конструктором по умолчанию,
- Типы с единственным пользовательским конструктором, требующим аргументы.

### Важные моменты

| Тип | `default_initializable`? |
|-----|-------------------------|
| `int`, `double` | ✅ Да |
| `std::vector<T>` | ✅ Да (пустой) |
| `std::unique_ptr<T>` | ✅ Да (nullptr) |
| `int&` | ❌ Нет — ссылку нельзя создать по умолчанию |
| `void` | ❌ Нет |
| `const int` | ❌ Нет — нужно инициализировать |
| `std::mutex` | ✅ Да |
| `std::thread` | ✅ Да (представляет неактивный поток) |

> 💡 Концепт полезен при работе с контейнерами, фабриками, алгоритмами.

### Лучшая практика

| Совет | Почему |
|------|--------|
| Используйте `std::default_initializable` вместо ручной проверки | Читаемее |
| Документируйте, где требуется создание по умолчанию | Особенно в API |
| Избегайте в местах, где достаточно `constructible_from<T, Args...>` | Более гибко |
| Предпочитайте для `value_type` в контейнерах | Например: `vector<T>(n)` |

```cpp
#include <iostream>
#include <concepts>

struct DefaultConstructible {
    int x, y;
};

struct NoDefaultConstructible {
    int x, y;

    NoDefaultConstructible(int _x, int _y):
        x{_x},
        y{_y} {}
};

template<std::default_initializable T>
T create() {
    T obj{};
    return obj;
}

template<typename T>
void test(T&& _input) {
    std::cout
        << typeid(_input).name()
        << std::endl;
}

int main() {
    test(create<int>());
    test(create<std::string>());
    test(create<DefaultConstructible>());
    // test(create<NoDefaultConstructible>()); // Error

    return 0;
}
```

```
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
struct DefaultConstructible
```
