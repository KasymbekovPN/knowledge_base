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

Переменной типа _bool_ присваивается значение другого типа. В этом случае переменная получает _false_, если значение равно 0. Во всех остальных случаях переменная получает _true_.
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    bool a = 1;
    bool b = 0;
    bool c = 'g';
    bool d = 3.4;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;
    cout << "d <= " << d << endl;

    return 0;
}
```

```
a <= 1
b <= 0
c <= 1
d <= 1
```

Числовой или символьной переменной присваивается значение типа _bool_. В этом случае переменная получает _1_, если значение равно _true_, либо получает _0_, если присваиваемое значение равно _false_.
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a = true;
    double b = false;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;

    return 0;
}
```

```
a <= 1
b <= 0
```

Целочисленной переменной присваивается дробное число. В этом случае дробная часть после запятой отбрасывается.
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a = 3.4;
    int b = 3.6;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;

    return 0;
}
```

```
a <= 3
b <= 3
```

Переменной, которая представляет тип с плавающей точкой, присваивается целое число. В этом случае, если целое число содержит больше битов, чем может вместить тип переменной, то часть информации усекается.
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    float a = 35005;
    double b = 3500500000033;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;

    return 0;
}
```

```
a <= 35005
b <= 3.5005e+12
```

Переменной беззнакового типа _unsigned_ присваивается значение не из его диапазона. В этом случае результатом будет остаток от деления по модулю. Например, тип _unsigned char_ может хранить значения от __0__ до __255__. Если присвоить ему значение вне этого диапазона, то компилятор присвоит ему остаток от деления по модулю __256__ (так как тип _unsigned char_ может хранить __256__ значений). Так, при присвоении значения __-5__ переменная типа unsigned char получит значение __251__
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    unsigned char a = -5;
    unsigned short b = -3500;
    unsigned int c = -50000000;

    cout << "a <= " << a << endl;
    cout << "b <= " << b << endl;
    cout << "c <= " << c << endl;

    return 0;
}
```

```
a <= √
b <= 62036
c <= 4244967296
```

Переменной знакового типа (_signed_) присваивается значение не из его диапазона. В этом случае результат не детерминирован. Программа может выдавать адекватный результат, а может работать некорректно.

---
[Статическая типизация и преобразования типов|metanit.com](https://metanit.com/cpp/tutorial/2.4.php)