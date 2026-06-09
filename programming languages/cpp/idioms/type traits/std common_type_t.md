---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/type traits/_|<=]]

Функция-тип **`std::common_type_t<T, U, ...>`** из заголовка `<type_traits>` определяет **наиболее подходящий общий тип**, к которому можно безопасно привести все заданные типы.

### Что делает `std::common_type_t<T, U...>`?

Он возвращает тип, который:
- Может хранить значения всех переданных типов,
- Обычно используется при смешанной арифметике, сравнении или выборе типа для результата.

✅ Примеры:
- `int` и `double` → `double`
- `long` и `unsigned int` → зависит от платформы, но часто `long long` или `unsigned long long`
- Указатели на `Base` и `Derived` → `Base*`

## ⚠️ Как вычисляется общий тип?

Правила:
1. Если один из типов — `void`, результат — неопределён.
2. Для арифметических типов: применяются стандартные **обычные арифметические преобразования** (usual arithmetic conversions).
3. Для классов: если есть перегруженный `operator?`, он может повлиять (редко).
4. Для указателей: возможен upcast (`Derived*` → `Base*`).

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename U>
void test();

int main() {
    test<int, double>();
    test<long, unsigned int>();
    test<float, int>();

    return 0;
}

template<typename T, typename U>
void test() {
    std::cout
        << "Common type of "
        << typeid(T).name()
        << " and "
        << typeid(U).name()
        << " is "
        << typeid(
            std::common_type_t<T, U>
        ).name() << std::endl;
}
```

```
Common type of int and double is double
Common type of long and unsigned int is unsigned long
Common type of float and int is float
```

```cpp
#include <iostream>
#include <type_traits>

template<typename T, typename U>
auto add(T _t, U _u) -> std::common_type_t<T, U> {
    return _t + _u;
}

int main() {
    auto&& result = add(3.14f, 42);
    std::cout << "value: " << result << std::endl;
    std::cout << "type: " << typeid(result).name() << std::endl;

    return 0;
}
```

```
value: 45.14
type: float
```
