---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::remove_all_extents_t<T>`** из заголовка `<type_traits>` удаляет **все уровни массива**, пока не останется базовый тип.

### Что делает `std::remove_all_extents_t<T>`?

Преобразует любой многомерный или одномерный массив в его **базовый элементарный тип**:

| Исходный тип | Результат |
|-------------|----------|
| `int[5]` → `int` |
| `double[3][4]` → `double` |
| `char[2][3][4]` → `char` |
| `int[]` → `int` (незавершённый массив) |
| `int` → `int` (если не массив — без изменений) |

✅ Это полезно, когда нужно получить "чистый" тип под любым количеством `[]`.

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename R>
void test();

int main() {
    std::cout << std::boolalpha;

    test<int[5], int>();
    test<double[3][4], double>();
    test<char[][2][3], char>();
    test<long, long>();

    return 0;
}

template<typename T, typename R>
void test() {
    std::cout
        << std::is_same_v<std::remove_all_extents_t<T>, R>
        << std::endl;
}
```

```
true
true
true
true
```
