---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - operator
  - return
---
[[__cpp syntax functions__|<==]]

Для возвращения результата из функции применяется оператор _return_. Этот оператор имеет две формы.

```cpp
return expression;
```

```cpp
return;
```

Первая форма оператора _return_ применяется для возвращения результата из функции. Если функция имеет в качестве возвращаемого типа любой тип, отличный от _void_, то такая функция обязательно должна возвратить некоторое значение с помощью оператора _return_. Причем возвращаемое значение должно соответствовать возвращаемому типу функции, либо допускать неявное преобразование в этот тип.

Единственная функция, которая возвращает некоторое значение, и где можно не использовать оператор _return_ - это функция _main_.

Например, мы хотим написать программу, которая бы вычисляла сумму чисел. Определим функцию, которая возвращает сумму чисел.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int calculate(const int, const int, const char);

int main(int argc, char const *argv[]) {
    const int result0 = calculate(1, 2, '+');
    const int result1 = calculate(1, 2, '-');

    cout << "result0 <= " << result0 << endl;
    cout << "result1 <= " << result1 << endl;

    return 0;
}

int calculate(const int a, const int b, const char op) {
    const int DEFAULT_RESULT = 0;
    switch (op) {
        case '+':
            return a + b;

        case '-':
            return a - b;

        default:
            return DEFAULT_RESULT;
    }
}
```

```
result0 <= 3
result1 <= -1
```

#### return без возвращения значения

Другая форма оператора _return_ не принимает после себя никаких значений и может использоваться в тех функциях, которые не возвращают никакого значения, то есть имеют в качестве возвращаемого типа _void_. Эта форма может быть полезна, если нам надо выйти из функции до ее завершения.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void func(const int, const std::string);

int main(int argc, char const *argv[]) {
    func(0, "first");
    func(1, "second");

    return 0;
}

void func(const int value, const std::string key) {
    cout << "[" << key << "] value <= " << value << endl;
    if (value <= 0) {
        return;
    }

    cout << "[" << key << "] it's positive " << endl;
}
```

```
[first] value <= 0
[second] value <= 1
[second] it's positive
```

---
[Оператор return и возвращение результата](https://metanit.com/cpp/tutorial/3.5.php)