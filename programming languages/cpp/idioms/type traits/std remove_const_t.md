---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип `std::remove_const_t<T>` из заголовка `<type_traits>` удаляет квалификатор `const` из типа `T`.
### Что делает `std::remove_const_t<T>`?

Преобразует:
- `const int` → `int`
- `const char*` → `char*`
- `const std::string&` → `std::string&`

⚠️ **Не убирает `const` из данных, на которые указывает указатель:**
- `const char*` → остаётся `const char*` (только указатель не `const`)
- Чтобы убрать `const` из всего, используйте `std::remove_cv_t`

### Важные моменты

| Исходный тип | После `remove_const_t`                                           |
| ------------ | ---------------------------------------------------------------- |
| `const int`  | `int`                                                            |
| `int`        | `int`                                                            |
| `const int*` | `const int*` ← указатель не был `const`, данные остались `const` |
| `int* const` | `int*` ← `const` убрали от указателя                             |
| `const int&` | `int&`                                                           |

> 💡 Чтобы убрать `const` со всего, что можно — часто используют:
```cpp
std::remove_cvref_t<T>
```
(удаляет `const`, `volatile`, ссылки)

### Лучшая практика

| Когда использовать                  | Как                                    |
| ----------------------------------- | -------------------------------------- |
| Нужно получить неконстантный тип    | `std::remove_const_t<T>`               |
| Работа с итераторами/указателями    | Комбинировать с другими трейтами       |
| Шаблонный код с `const` параметрами | Извлекать базовый тип перед сравнением |
| Обработка универсальных ссылок      | Лучше: `std::remove_cvref_t<T>`        |

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void test_remove_const_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_const_t<T>).name()
        << std::endl;
}

template<typename T>
void test_remove_cv_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_cv_t<T>).name()
        << std::endl;
}

template<typename T>
void test_remove_cvref_t() {
    std::cout
        << typeid(T).name() << ": "
        << typeid(std::remove_cvref_t<T>).name()
        << std::endl;
}

int main() {
    std::cout << std::boolalpha;

    test_remove_const_t<const int>();
    test_remove_const_t<int>();
    test_remove_const_t<const double>();
    test_remove_const_t<const int*>();
    test_remove_const_t<int* const>();
    test_remove_const_t<const char*>();

    test_remove_cv_t<const volatile int>();

    test_remove_cvref_t<const std::string&>();

    return 0;
}
```

```
int: int
int: int
double: double
int const * __ptr64: int const * __ptr64
int * __ptr64: int * __ptr64
char const * __ptr64: char const * __ptr64
int: int
class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >: class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >
```
