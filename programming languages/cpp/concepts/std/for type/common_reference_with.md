---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::common_reference_with<T, U>`** из заголовка `<concepts>` проверяет, существует ли **общая ссылочная форма** для типов `T` и `U`, к которой можно неявно преобразовать оба типа.

### Что такое "общая ссылочная форма"?

Это означает:  
Существует такой тип `C`, что:
- `T&` можно неявно преобразовать в `C`,
- `U&` можно неявно преобразовать в `C`.

✅ Это используется, например, при сравнении значений разных типов или передаче в функции, которые работают с общим типом.

### Важные моменты

| Условие | Результат |
|--------|----------|
| `T` и `U` — одинаковые ссылки | ✅ `true` |
| `T = int&`, `U = const int&` | ✅ `true` → `const int&` |
| `T = double&`, `U = int&` | ❌ Зависит от контекста, обычно нет |
| `T = A*`, `U = B*` | ❌ Нет, если не связаны наследованием |

> 💡 Часто используется внутри библиотечных компонентов STL (например, алгоритмы сравнения).

### Лучшая практика

| Совет                                                          | Почему                                            |
| -------------------------------------------------------------- | ------------------------------------------------- |
| Используйте `common_reference_with` в библиотечном коде        | Для поддержки смешанных типов                     |
| Не путайте с `convertible_to`                                  | То первое — про **ссылки**, второе — про значения |
| Документируйте требования к типам                              | Особенно при создании своих concept'ов            |
| Предпочитайте `same_as` или `derived_from` для простых случаев | Более понятно                                     |

```cpp
#include <iostream>
#include <concepts>

struct Number {
    int value;

    operator int() const { return value; }
};

template<typename T, typename U>
void test() {
    if constexpr (std::common_reference_with<T, U>) {
        std::cout
            << "Common refertence types exist: "
            << typeid(
                std::common_reference_t<T, U>
            ).name();
    } else {
        std::cout
            << "No common reference between types";
    }
    std::cout << std::endl;
}

int main() {
    test<int, int>();
    test<float, double>();
    test<Number, int>();
    test<int, std::string>();

    return 0;
}
```

```
Common refertence types exist: int
Common refertence types exist: double
Common refertence types exist: int
No common reference between types
```

