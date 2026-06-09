---
tags:
  - programming-language
  - cpp
  - syntax
  - construction
  - if
---
[[__cpp syntax constractions__|<=]]

#### if

Условная конструкция _if_-_else_ направляет ход программы по одному из возможных путей в зависимости от условия. Она проверяет истинность условия, и если оно истинно, выполняет блок инструкций. В простейшем виде конструкция _if_ имеет следующую сокращенную форму

```
if (condition)
{
	instruction;
}
```

В качестве условия использоваться условное выражение, которое возвращает _true_ или _false_. Если условие возвращает _true_, то выполняются последующие инструкции, которые входят в блок _if_. Если условие возвращает _false_, то последующие инструкции не выполняются. Блок инструкций заключается в фигурные скобки. Например:
```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{8};

    if (a == 8) {
        cout << "a == 8" << endl;
    }
    cout << "End of programm" << endl;

    return 0;
}
```

```
a == 8
End of programm
```

#### if-else

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int n{21};

    if (n > 21) {
        cout << "n > 21" << endl;
    } else {
        cout << "n <= 21" << endl;
    }

    return 0;
}
```

```
n <= 21
```

#### if-elseif-else

Однако нередко надо обработать не два возможных альтернативных варианта, а гораздо больше.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int n{21};

    if (n > 21) {
        cout << "n > 21" << endl;
    } else if (n == 21) {
        cout << "n == 21" << endl;
    } else {
        cout << "n < 21" << endl;
    }

    return 0;
}
```

```
n == 21
```

Если в блоке _if_ или _else_ или _else_-_if_ необходимо выполнить только одну инструкцию, то фигурные скобки можно опустить.

#### Целочисленные условия

Стоит отметить, что если вместо значений типа _bool_ передаются целые числа, то они преобразуются к типу _bool_ - для нулевых значений возвращается _false_, для ненулевых - _true_

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{8};
    int b{};

    if (a) {
        cout << "a likes true" << endl;
    } else {
        cout << "a likes false" << endl;
    }

    if (b) {
        cout << "b likes true" << endl;
    }
    else {
        cout << "b likes false" << endl;
    }

    return 0;
}
```

```
a likes true
b likes false
```

#### Блок if с инициализацией переменной

Иногда в конструкции _if_ для различных промежуточных вычислений необходимо определить переменную. Мы можем это сделать непосредственно в блоке кода. Однако начиная со стандарта __C++17__ язык __С++__ поддерживает особую форму конструкции _if_:

```
if (initialization; condition) {
	instruction;
}
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int a{5};
    int b{3};

    if (int c{a-b}; a > b) {
        cout << "true <> " << c << endl;
    } else {
        cout << "false <> " << c << endl;
    }

    if (int rem {a % b}; rem == 0) {
        cout << "true <> " << rem << endl;
    } else {
        cout << "false <> " << rem << endl;
    }

    return 0;
}
```

```
true <> 2
false <> 2
```

---
[Конструкции](https://metanit.com/cpp/tutorial/2.12.php)