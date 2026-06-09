---
tags:
  - programming-language
  - cpp
  - syntax
  - function
---
[[__cpp syntax functions__|<==]]

Аргументы, которые представляют переменные или константы, могут передаваться в функцию по значению (by value) и по ссылке (by reference).

#### Передача аргументов по значению

При передаче аргументов по значению функция получает копию значения переменных и констант. 

```cpp
#include <iostream>

using std::cout;
using std::endl;

void square(int);

int main(int argc, char const *argv[]) {
    int number {42};
    cout << "Before <= " << number << endl;

    square(number);
    cout << "After <= " << number << endl;

    return 0;
}

void square(int n) {
    n = n*n;
    cout << "[square] n <= " << n << endl;
}
```

```
Before <= 42
[square] n <= 1764
After <= 42
```

#### Передача параметров по ссылке

При передаче параметров по ссылке передается ссылка на объект, через которую мы можем манипулировать самим объектов, а не просто его значением.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void square(int&);

int main(int argc, char const *argv[]) {
    int number {42};
    cout << "Before <= " << number << endl;

    square(number);
    cout << "After <= " << number << endl;

    return 0;
}

void square(int& n) {
    n = n*n;
    cout << "[square] n <= " << n << endl;
}
```

```
Before <= 42
[square] n <= 1764
After <= 1764
```

Передача по ссылке позволяет возвратить из функции сразу несколько значений. Также передача параметров по ссылке является более эффективной при передаче очень больших объектов. Поскольку в этом случае не происходит копирования значений, а функция использует сам объект, а не его значение.

От передачи аргументов по ссылке следует отличать передачу ссылок в качестве аргументов. Если функция принимает аргументы по значению, то изменение параметров внутри функции также никак не скажется на внешних объектах, даже если при вызове функции в нее передаются ссылки на объекты.

Передача параметров по значению больше подходит для передачи в функцию небольших объектов, значения которых копируются в определенные участки памяти, которые потом использует функция.

Передача параметров по ссылке больше подходит для передачи в функцию больших объектов, в этом случае не нужно копировать все содержимое объекта в участок памяти, за счет чего увеличивается производительность программы.

#### Преобразования типов

Передача параметров по значению и по ссылке отличаются еще одним важным моментом. __С++__ может автоматически преобразовывать значения одних типов в другие, в том числе если подобные преобразования сопровождаются потерей точности (например, преобразование от типа _double_ к типу _int_). Но при передаче параметров по ссылке неявные автоматические преобразования типов исключены.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void print_val(int);
void print_ref(int&);

int main(int argc, char const *argv[]) {
    double number {3.14159};

    print_val(number);
    print_ref(number); // Error

    return 0;
}

void print_val(int n){
    cout << "[print_val] n <= " << n << endl;
}

void print_ref(int& n) {
    cout << "[print_ref] n <= " << n << endl;
}
```

```
.\arguments_by_val_and_ref_with_cast.cpp:13:5: error: no matching function for call to 'print_ref'
   13 |     print_ref(number);
      |     ^~~~~~~~~
.\arguments_by_val_and_ref_with_cast.cpp:7:6: note: candidate function not viable: no known conversion from 'double' to 'int &' for 1st argument
    7 | void print_ref(int&);
      |      ^         ~~~~
```

---
[Передача аргументов по значению и по ссылке](https://metanit.com/cpp/tutorial/3.3.php)