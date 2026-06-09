---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::underlying_type_t<T>`** из заголовка `<type_traits>` позволяет получить **базовый (fundamental) тип**, на котором основан перечислимый тип (`enum`).

### Что делает `std::underlying_type_t<T>`?

Он возвращает тип, используемый компилятором для хранения значений `enum`.

Пример:
```cpp
enum Color { Red, Green, Blue }; // базовый тип обычно int
```
→ `std::underlying_type_t<Color>` → `int`

✅ Это полезно при:
- Преобразовании `enum` в целое,
- Сериализации,
- Работе с битовыми флагами,
- API, где нужно передать `enum` как число.

## ⚠️ Важные моменты

| Ситуация | Результат |
|---------|----------|
| Обычный `enum` без указания типа | Обычно `int` |
| `enum class : short` | `short` |
| `enum class` без базового типа | Обычно `int` |
| Не `enum` (например, `int`) | ❌ `underlying_type_t` не определён → ошибка компиляции |

> 💡 Используйте `std::is_enum_v<T>` для проверки до вызова.

```cpp
#include <iostream>
#include <type_traits>

enum Status {
    Idle,
    Running,
    Stopped
};

enum Priority : uint8_t {
    Low = 33,
    Medium,
    High
};

template<typename T>
void test(const T&&);

int main() {
    test<Status>(Status::Running);
    test<Priority>(Priority::Low);

    std::cout
        << "to_underlying: "
        << std::to_underlying(Status::Stopped)
        << std::endl;

    return 0;
}

template<typename T>
void test(const T&& _value) {
    std::cout
        << "{ " << _value
        << ", " << typeid(T).name()
        << ", " << typeid(std::underlying_type_t<T>).name()
        << "}" << std::endl;
}
```

```
{ 1, enum Status, int}
{ !, enum Priority, unsigned char}
to_underlying: 2
```
