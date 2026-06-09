---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for creation/_|<=]]

Концепт **`std::constructible_from<T, Args...>`** из заголовка `<concepts>` проверяет, можно ли **создать объект типа `T`** из аргументов типов `Args...`.

### Что означает `std::constructible_from<T, Args...>`?

✅ То есть:
- У `T` есть конструктор, принимающий `Args...`,
- И он не `= delete`,
- И доступен (не `private`),
- И не вызывает ошибки SFINAE.

### Важные моменты

| Выражение | Результат |
|----------|-----------|
| `std::constructible_from<int, int>` | ✅ `true` |
| `std::constructible_from<int, double>` | ✅ `true` (неявное преобразование) |
| `std::constructible_from<std::string, int>` | ❌ `false` |
| `std::constructible_from<std::vector<int>, size_t, int>` | ✅ `true` (конструктор count + value) |
| `std::constructible_from<int&, int>` | ❌ `false` (ссылку нельзя создать так) |

> 💡 Для ссылок используйте `std::convertible_to` или `std::is_convertible_v`.

### Лучшая практика

| Совет                                                                | Почему                              |
| -------------------------------------------------------------------- | ----------------------------------- |
| Используйте `constructible_from` вместо `std::is_constructible_v`    | Читаемее и современнее              |
| Документируйте требования к типам                                    | Особенно в библиотечном коде        |
| Избегайте слишком широких ограничений                                | Может пропустить нежелательные типы |
| Предпочитайте его при работе с фабриками, контейнерами, аллокаторами | Точный контроль                     |

```cpp
#include <iostream>
#include <concepts>
#include <vector>
#include <string>

template<typename T, typename... Args>
requires std::constructible_from<T, Args...>
T create(Args&&... _args) {
    return T(std::forward<Args>(_args)...);
}

template<typename T>
void test(T&& _input) {
    std::cout
        << typeid(_input).name()
        << std::endl;
}

int main() {
    test(create<int>(42));
    test(create<std::string>("42"));
    test(create<std::vector<int>>(std::vector<int>({1, 2, 3})));

    return 0;
}
```

```
int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
class std::vector<int,class std::allocator<int> >
```
