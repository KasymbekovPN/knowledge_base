---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::enable_if_t<Condition, T>`** из заголовка `<type_traits>` используется для **условного включения шаблонной функции или класса** на этапе компиляции (SFINAE — Substitution Failure Is Not An Error).
### Что делает `std::enable_if_t<C, T>`?

Он работает так:

| Условие | Результат |
|--------|-----------|
| `C == true` → `T` |
| `C == false` → **неопределённый тип**, что приводит к отбрасыванию перегрузки (SFINAE) |

✅ Это позволяет создавать **перегрузки только для определённых типов**.

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
std::enable_if_t<std::is_integral_v<T>, int>
test(T);

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, float>
test(T);

int main(int argc, char const *argv[]) {
    std::cout
        << "int output"
        << test(12)
        << std::endl;

    std::cout
        << "float output"
        << test(4.2f)
        << std::endl;

	// error
    // std::cout
    //     << "string output"
    //     << test("hello")
    //     << std::endl;

    return 0;
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, int>
test(T _input) {
    std::cout << "int input: " << _input << std::endl;
    return _input * _input;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, float>
test(T _input) {
    std::cout << "float input: " << _input << std::endl;
    return _input * _input;
}
```

```
int outputint input: 12
144
float outputfloat input: 4.2
17.64
```


```
error: no matching function for call to 'test'
```
