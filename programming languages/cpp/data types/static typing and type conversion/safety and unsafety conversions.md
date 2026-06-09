---
tags:
  - programming-language
  - cpp
  - data-types
  - static
  - cast
  - conversion
---
[[__cpp data types index__|<=]]

Те преобразования, при которых не происходит потеря информации, являются безопасными. Как правило, это преобразования от типа с меньшей разрядностью к типу с большей разрядностью. В частности, это следующие цепочки преобразований:

_bool_ -> _char_ -> _short_ -> _int_ -> _double_ -> _long double_

_bool_ -> _char_ -> _short_ -> _int_ -> _long_ -> _long long_

_unsigned char_ -> _unsigned short_ -> _unsigned int_ -> _unsigned long_

_float_ -> _double_ -> _long double_

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    cout << "SAFETY" << endl;
    short a = 'g';  // char -> short
    int b = 10;
    double c = b;   // int -> double
    float d = 3.4;
    double e = d;   // float -> double
    double f = 35;  // int -> double
    cout << "a <= "  << a << endl;
    cout << "c <= "  << c << endl;
    cout << "e <= "  << e << endl;
    cout << "f <= "  << f << endl;

    cout << "UNSAFETY" << endl;
    unsigned int u0 = -25;
    unsigned short u1 = -3500;
    cout << "u0 <= " << u0 << endl;
    cout << "u1 <= " << u1 << endl;

    // ERROR
    // unsigned int e0 {-25};
    // unsigned short e1 {-3500};

    return 0;
}
```

```
SAFETY
a <= 103
c <= 10
e <= 3.4
f <= 35
UNSAFETY
u0 <= 4294967271
u1 <= 62036
```

Многое зависит от компилятора. В ряде случаев компиляторы при компиляции выдают предупреждение, тем не менее программа может быть успешно скомпилирована. В других случаях компиляторы не выдают никакого предупреждения. 

Собственно в этом и заключается опасность, что программа успешно компилируется, но тем не менее существует риск потери точности данных. Значение переменной - это всего лишь набор битов в памяти, которые интерпретируются в соответствии с определенным типом. И для разных типов один и тот же набор битов может интерпретироваться по разному. Поэтому важно учитывать диапазоны значений для того или иного типа при присвоении переменной значения.

Если речь идет об инициализации переменных, то, чтобы избежать опасных преобразований, когда может произойти потеря точности, рекомендуется использовать инициализацию в фигурных скобках:

В этом случае компилятор сгенерирует ошибку, и программа не скомпилируется.

```
error: constant expression evaluates to -25 which cannot be narrowed to type 'unsigned int' [-Wc++11-narrowing]
error: constant expression evaluates to -3500 which cannot be narrowed to type 'unsigned short' [-Wc++11-narrowing]
```
---
[Статическая типизация и преобразования типов|metanit.com](https://metanit.com/cpp/tutorial/2.4.php)