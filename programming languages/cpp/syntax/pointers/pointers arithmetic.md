---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
---
[[__cpp syntax pointers__|<==]]

Указатели могут участвовать в арифметических операциях (сложение, вычитание, инкремент, декремент). Однако сами операции производятся немного иначе, чем с числами. И многое здесь зависит от типа указателя.

К указателю можно прибавлять целое число, и также можно вычитать из указателя целое число. Кроме того, можно вычитать из одного указателя другой указатель.

#### Инкремент, декремент
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number{42};
    int* pnumber{&number};

    cout << "pnumber <= " << pnumber << endl;
    pnumber++;
    cout << "pnumber <= " << pnumber << endl;
    pnumber--;
    cout << "pnumber <= " << pnumber << endl;

    return 0;
}
```

```
pnumber <= 000000FE62AFFA30
pnumber <= 000000FE62AFFA34
pnumber <= 000000FE62AFFA30
```

Операция инкремента `++` увеличивает значение на единицу. В случае с указателем увеличение на единицу будет означать увеличение адреса, который хранится в указателе, на размер типа указателя. Декремент - по аналогии.

#### Сложение, вычитание

Аналогично указатель будет изменяться при прибавлении/вычитании не единицы, а какого-то другого числа.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    double number {14.6};
    double* pdouble {&number};

    cout << "pdouble <= " << pdouble << endl;
    pdouble += 2;
    cout << "pdouble <= " << pdouble << endl;

    short sh {5};
    short* psh {&sh};

    cout << "psh <= " << psh << endl;
    psh -= 3;
    cout << "psh <= " << psh << endl;

    return 0;
}
```

```
pdouble <= 000000DB31FEF8C8
pdouble <= 000000DB31FEF8D8
psh <= 000000DB31FEF8BE
psh <= 000000DB31FEF8B8
```

В случае _pdouble_ - увеличение на `16 = 2 * 8`
В случае _psh_ - уменьшение на `6 = 3 * 2`

В отличие от сложения операция вычитания может применяться не только к указателю и целому числу, но и к двум указателям одного типа.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numa {7};
    int numb {8};
    int* pnuma {&numa};
    int* pnumb {&numb};

    auto sub {pnuma - pnumb};
    cout << pnuma << " - " << pnumb << " = " << sub << endl;

    return 0;
}
```

```
00000011648FF810 - 00000011648FF80C = 1
```

Согласно стандарту разность указателей представляет тип `std::ptrdiff_t`, который в реальности является псевдонимом для типов `int`, `long` и `long long`. Какой конкретно из этих типов применяется для хранения разности, зависит от конкретной платформы. Например, на Windows 64x это тип `long long`.

#### Некоторые особенности операций

При работе с указателями надо отличать операции с самим указателем и операции со значением по адресу, на который указывает указатель.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int number0 {10};
    int* pnumber0 {&number0};
    int result0 {*pnumber0 + 1};
    cout << "result0 <= " << result0 << endl;

    int number1 {20};
    int* pnumber1 {&number1};
    int result1 {++*pnumber1};
    cout << "*pnumber1 <= " << *pnumber1 << endl;
    cout << "result1 <= " << result1 << endl;

    int number2 {30};
    int* pnumber2 {&number2};
    int result2 {*pnumber2++};
    cout << "*pnumber2 <= " << *pnumber2 << endl;
    cout << "result2 <= " << result2 << endl;

    return 0;
}
```

```
result0 <= 11
*pnumber1 <= 21
result1 <= 21
*pnumber2 <= 21
result2 <= 30
```

- Постфиксный инкремент/декремент имеют приоритет над разыменовыванием.
- Префиксный инкремент/декремент имеют меньший приоритет, чем над разыменовывание.

---
[Арифметика указателей](https://metanit.com/cpp/tutorial/4.3.php)