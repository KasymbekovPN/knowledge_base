---
tags:
  - programming-language
  - cpp
  - concepts
---
[[programming languages/cpp/concepts/std/for type/_|<=]]

Концепт **`std::floating_point<T>`** из заголовка `<concepts>` проверяет, что тип является **числом с плавающей точкой**.
## 📘 Что означает `std::floating_point<T>`?

Тип `T` удовлетворяет концепту, если он:
- Представляет число с плавающей точкой,
- Обеспечивает семантику IEEE 754 (или аналогичную),
- Поддерживает операции: `+`, `-`, `*`, `/`, сравнение, NaN, бесконечность и т.д.

✅ Включает:
- `float`
- `double`
- `long double`

❌ Не включает:
- Целочисленные типы (`int`, `short`, `size_t`)
- Фиксированную точку (если реализована как класс)
- `bool`, указатели, массивы

### Важные моменты

| Тип                         | Удовлетворяет `std::floating_point`? |
| --------------------------- | ------------------------------------ |
| `float`                     | ✅ Да                                 |
| `double`                    | ✅ Да                                 |
| `long double`               | ✅ Да (зависит от реализации)         |
| `int`, `unsigned long`      | ❌ Нет                                |
| `std::complex<float>`       | ❌ Нет (это класс, не скаляр)         |
| `half` (внешние библиотеки) | ❌ Нет, если не специализировано      |

> 💡 Концепт строго определён на фундаментальных типах C++.

### Лучшая практика

| Совет | Почему |
|------|--------|
| Используйте `std::floating_point` вместо `std::is_floating_point_v<T>` | Более читаемо и безопасно |
| Документируйте, где требуется точность и диапазон | Особенно в математических функциях |
| Избегайте смешивания `float` и `double` без явного преобразования | Потеря точности |
| Предпочитайте `double` по умолчанию | `float` — для экономии памяти |

```cpp
#include <iostream>
#include <concepts>

template<std::floating_point T>
void test(T);

int main(int argc, char const *argv[]) {
    test(3.14f);
    test(2.718);
    test(1.414L);

    // test(42); // Error
    // test("hello"); // Error

    return 0;
}

template<std::floating_point T>
void test(T _input) {
    std::cout << "Input: " << _input << std::endl;
}
```


```
Input: 3.14
Input: 2.718
Input: 1.414
```
