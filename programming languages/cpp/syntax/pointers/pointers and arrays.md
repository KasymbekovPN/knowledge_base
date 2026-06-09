---
tags:
  - programming-language
  - cpp
  - syntax
  - pointer
  - array
---
[[__cpp syntax pointers__|<==]]

В __C++__ указатели и массивы тесно связаны. Обычно компилятор преобразует массив в указатели. С помощью указателей можно манипулировать элементами массива, как и с помощью индексов.

Имя массива по сути является адресом его первого элемента. Соответственно через операцию разыменования мы можем получить значение по этому адресу.

Прибавляя к адресу первого элемента некоторое число, мы можем получить определенный элемент массива.

В отношении сложения и вычитания здесь действуют те же правила, что и в операциях с указателями. Добавление единицы означает прибавление к адресу значения, которое равно размеру типа массива. Так, в данном случае массив представляет тип int, размер которого, как правило, составляет _4_ байта, поэтому прибавление единицы к адресу означает увеличение адреса на _4_. Прибавляя к адресу _2_, мы увеличиваем значение адреса на _4_ * _2_ = _8_. И так далее.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2, 3, 4};
    cout << "numbers[0] address <= " << numbers << endl;
    cout << "numbers[0] value   <= " << *numbers << endl;
    cout << "---" << endl;

    for (size_t i = 0; i < std::size(numbers); i++){
        cout
	        << "numbers[" << i << "] | address <= "
	        << numbers + i << " | value <= "
	        << *(numbers + i) << endl;
    }
    return 0;
}
```

```
numbers[0] | address <= 000000B92874FDA0 | value <= 0
numbers[1] | address <= 000000B92874FDA4 | value <= 1
numbers[2] | address <= 000000B92874FDA8 | value <= 2
numbers[3] | address <= 000000B92874FDAC | value <= 3
numbers[4] | address <= 000000B92874FDB0 | value <= 4
```

Но при этом имя массива это не стандартный указатель, и мы не можем изменить его адрес.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2};
    numbers++;

    return 0;
}
```

```
.\pointers_and_arrays_common_bad.cpp:9:12: error: cannot increment value of type 'int[3]'
    8 |     numbers++;
      |     ~~~~~~~^
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int other {42};
    int numbers[] {0, 1, 2};
    numbers = &other;

    return 0;
}
```

```
.\pointers_and_arrays_common_bad.cpp:9:13: error: array type 'int[3]' is not assignable
    9 |     numbers = &other;
      |     ~~~~~~~ ^
```

#### Указатели на массивы

Имя массива всегда хранит адрес самого первого элемента. И нередко для 
перемещения по элементам массива используются отдельные указатели.

Можно сразу присвоить указателю адрес конкретного элемента массива. С помощью указателей легко перебрать массив. Так как указатель хранит адрес, то мы можем продолжать цикл, пока адрес в указателе не станет равным адресу последнего элемента. Аналогичным образом можно перебрать и многомерный массив.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2, 3, 4};
    int* pointer {numbers};
    int number2 {*(pointer + 2)};
    cout << "number2 <= " << number2 << endl;

    int* pointer2 {&numbers[2]};
    cout << "*pointer2 <= " << *pointer2 << endl;

    for (int* ptr{numbers}; ptr <= &numbers[4]; ptr++) {
        cout << "addr <= " << ptr << " | value <= " << *ptr << endl;
    }

    return 0;
}
```

```
number2 <= 2
*pointer2 <= 2
addr <= 0000002498BBFE60 | value <= 0
addr <= 0000002498BBFE64 | value <= 1
addr <= 0000002498BBFE68 | value <= 2
addr <= 0000002498BBFE6C | value <= 3
addr <= 0000002498BBFE70 | value <= 4
```

#### Указатель на строки и массивы символов

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char hello[] {"hello"};
    char* phello {hello};

    cout << "phello  <= " << phello << endl;
    cout << "address <= " << (void*)phello << endl;

    return 0;
}
```

```
phello  <= hello
address <= 000000C03BAFF6DE
```

Поскольку массив символов может интерпретироваться как строка, то указатель на значения типа char тоже может интерпретироваться как строка.

При выводе на консоль значения указателя фактически будет выводиться строка.

Однако следует учитывать, что строковые литералы в С++ рассматриваются как константы. Поэтому при определении указателя на строку, следует определять указатель как указатель на константу

#### Массивы указателей

Также можно определять массивы указателей. В некотором смысле массив указателей будет похож на массив, который содержит другие массивы. Однако массив указателей имеет преимущества.

Для определения двухмерного массива мы должны указать как минимум размер вложенных массивов, который будет достаточным, чтобы вместить каждую строку. В данном случае размер каждого вложенного массива - _20"_ символов. Однако зачем для первой строки - "C++", которая содержит _4_ символа (включая концевой нулевой байт) выделять аж _20_ байтов? Это - ограничение подобных массивов. Массивы указателей же позволяют обойти подобное ограничение.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    char langs[][20] = {"C++", "Python", "JavaScript"};
    cout << "Size of langs[0] <= " << std::size(langs[0])
	     << " bytes" << endl;

    const char* plangs[] {"C++", "Python", "JavaScript"};
    for (size_t i{}; i < std::size(plangs); i++) {
        cout << "'" << plangs[i] << "'" << endl;
    }

    return 0;
}
```

```
Size of langs[0] <= 20 bytes
'C++'
'Python'
'JavaScript'
```

---
[Указатели и массивы](https://metanit.com/cpp/tutorial/4.5.php)