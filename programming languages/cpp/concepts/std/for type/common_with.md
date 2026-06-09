---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::common_with<T, U>`** из заголовка `<concepts>` проверяет, существует ли **общий тип**, к которому можно неявно преобразовать **оба типа `T` и `U`**, и при этом этот общий тип сам может быть использован в выражениях.

### Что делает `std::common_with<T, U>`?

Он возвращает `true`, если:
1. Существует тип `C`, такой что:
   - `T` можно неявно привести к `C`,
   - `U` можно неявно привести к `C`,
2. И при этом: `C{std::declval<T>()}` и `C{std::declval<U>()}` — корректные выражения,
3. То есть `C` является **валидным общим типом** для обоих.

✅ Это более строгое условие, чем просто "можно преобразовать", потому что требует **консистентности и конструктуруемости**.

### Важные моменты

| Выражение | Результат |
|----------|----------|
| `std::common_with<int, long>` | ✅ Зависит от платформы, но часто `true` |
| `std::common_with<float, double>` | ✅ `double` — общий тип |
| `std::common_with<int*, void*>` | ❌ Нет общего типа (`void*` не может быть сконструирован из `int*` в строгом смысле?) |
| `std::common_with<std::vector<int>, std::list<int>>` | ❌ Нет общего типа |

> 💡 `common_with` часто используется внутри STL, например:
- В алгоритмах сравнения,
- В `std::variant`,
- При выводе типов в `std::common_type`.

### Лучшая практика

| Совет                                                          | Почему                                           |
| -------------------------------------------------------------- | ------------------------------------------------ |
| Используйте `common_with` для смешанных операций               | Например, `min(a, b)` где `a` и `b` разных типов |
| Не полагайтесь на него для указателей без иерархии             | Может не сработать                               |
| Документируйте ожидаемое поведение                             | Особенно в библиотечном коде                     |
| Предпочитайте `same_as` или `derived_from` для простых случаев | Более понятно                                    |

```cpp
#include <iostream>
#include <concepts>

template<typename T, typename U>
requires std::common_with<T, U>
void test(const T&, const U&);

int main(){
    test(42, 3.14159);
    test("hi", std::string("hello"));

    return 0;
}

template<typename T, typename U>
requires std::common_with<T, U>
void test(const T& _t, const U& _u) {
    std::cout
        << "Both can be used as type: "
        << typeid(
            std::common_type_t<T, U>
        ).name()
        << std::endl;
}
```

```
Both can be used as type: double
Both can be used as type: class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
```
