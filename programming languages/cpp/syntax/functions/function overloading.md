---
tags:
  - programming-language
  - syntax
  - cpp
  - function
  - overloading
---
[[__cpp syntax functions__|<==]]

Язык __С++__ позволяет определять функции с одним и тем же именем, но разным набором параметров. Подобная возможность называется перегрузкой функций (_function_ _overloading_). Компилятор же на этапе компиляции на основании параметров выберет нужный тип функции.

Чтобы определить несколько различных версий функции с одним и тем же именем, все эти версии должны отличаться как минимум по одному из следующих признаков:
- имеют разное количество параметров
- соответствующие параметры имеют разный тип

При этом различные версии функции могут также отличаться по возвращаемому типу. Однако компилятор, когда выбирает, какую версию функции использовать, ориентируется именно на количество параметров и их тип.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int sum(int, int);
int sum(int, int, int);
double sum(double, double);

int main(int argc, char const *argv[]) {
    const double d0 {3.3};
    const double d1 {4.4};
    const double d2 {5.5};

    cout << "variant 0: <= " << sum((int) d0, (int) d1) << endl;
    cout << "variant 1: <= " << sum(d0, d1, d2) << endl;
    cout << "variant 2: <= " << sum(d0, d1) << endl;

    return 0;
}

int sum(int a0, int a1) {
    return a0 + a1;
}

int sum(int a0, int a1, int a2) {
    return a0 + a1 + a2;
}

double sum(double a0, double a1) {
    return a0 + a1;
}
```

#### Перегрузка функций и параметрами-ссылки

При перегрузке функций с параметрами-ссылками следует учитывать, что параметры типов _data_type_ и _data_type&_ не различаются при перегрузке. 

Например, два следующих прототипа (для _int_) не считаются разными версиями функции _print_.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int print(int);
int print(int&);
int print(double);

int main(int argc, char const *argv[]) {
    const int number {42};

    print(number);
    print(&number);
    print((double) number);

    return 0;
}

int print(int number) {
    cout << "[print(int)] " << number << endl;
}

int print(int& number) {
    cout << "[print(int&)] " << number << endl;
}

int print(double number) {
    cout << "[print(double)] " << number << endl;
}
```

```
.\func_overloading_and_ref_param.cpp:14:5: error: no matching function for call to 'print'
   14 |     print(&number);
```

#### Перегрузка и параметры-константы

При перегрузке функций константный параметр отличается от не константного параметра только для ссылок и указателей. В остальных случаях константный параметр будет идентичен не константному параметру. 

Например, следующие два прототипа при перегрузке различаться __НЕ__ будут:

```cpp
void print(int);
void print(const int);
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int square(const int*);
int square(int*);

int main(int argc, char const *argv[]){
    const int cnumber {42};
    int number {43};

    cout << "square(&cnumber) <= " << square(&cnumber) << endl;
    cout << "square(&number) <= " << square(&number) << endl;

    return 0;
}

int square(const int* p_number) {
    return *p_number * *p_number;
}

int square(int* p_number) {
    *p_number = *p_number * *p_number;

    return *p_number;
}
```

```
square(&cnumber) <= 1764
square(&number) <= 1849
```

---
[Перегрузка функций](https://metanit.com/cpp/tutorial/4.14.php)