---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - array
---
[[__cpp syntax functions__|<==]]

Если функция принимает в качестве параметра массив, то фактически в эту функцию передается указатель на первый элемент массива. То есть как и в случае с указателями нам доступен адрес, по которому мы можем менять значения. Поэтому следующие объявления функции будут по сути равноценны.

```cpp
void func(int numbers[]);
```

```cpp
void func(int* numbers);
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

void func_as_array(const int numbers[]);
void func_as_ptr(const int* const ptr);

int main(int argc, char const *argv[]) {
    const int numbers[] {0, 1, 2};

    func_as_array(numbers);
    func_as_ptr(numbers);

    return 0;
}

void func_as_array(const int numbers[]) {
    cout << "numbers[0] <= " << numbers[0] << endl;
}

void func_as_ptr(const int* const ptr) {
    cout << "*ptr <= "  << *ptr << endl;
}
```

```
numbers[0] <= 0
*ptr <= 0
```

#### Ограничения

Поскольку параметр, определенный как массив, рассматривается именно как указатель на первый элемент, то мы не сможем корректно получить длину массива, например, следующим образом. А также мы не сможем использовать цикл for для перебора этого массива.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void test_sizeof(int numbers[]);
void test_size(int numbers[]);
void test_for(int numbers[]);

int main(int argc, char const *argv[]) {
    int numbers[] {0, 1, 2};

    test_sizeof(numbers);
    test_size(numbers);
    test_for(numbers);

    return 0;
}

void test_sizeof(int numbers[]) {
    int size = sizeof(numbers); // Warning
    cout << "[test_sizeof] " << size << endl;
}

void test_size(int numbers[]) {
    // size_t size = std::size(numbers); // Error
}

void test_for(int numbers[]) {
    // for (int n: numbers) {} // Error
}
```

```
[test_sizeof] 8
```

```
.\array_as_func_params_restrictions.cpp:21:22: warning: sizeof on array function parameter will return size of 'int *' instead of 'int[]' [-Wsizeof-array-argument]
   21 |     int size = sizeof(numbers); // Warning
```

```
.\array_as_func_params_restrictions.cpp:26:19: error: no matching function for call to 'size'
   26 |     size_t size = std::size(numbers); // Error
      |                   ^~~~~~~~~
```

```
.\array_as_func_params_restrictions.cpp:30:17: error: cannot build range expression with array function parameter 'numbers' since parameter with array type 'int[]' is treated as   
      pointer type 'int *'
   30 |     for (int n: numbers) {} // Error
```

#### Передача маркера конца массива

Чтобы должным образом определять конец массив, перебирать элементы массива, обращаться к этим элементам, необходимо использовать специальный маркер, который бы сигнализировал об окончании массива. Для этого могут использоваться разные подходы.

Первый подход заключается в том, чтобы один из элементов массива сам сигнализировал о его окончании. В частности, массив символов может представлять строку - набор символов, который завершается нулевым символом `\0`. Фактически нулевой символ служит признаком окончания символьного массива.

Однако при таком подходе мы должны быть уверены, что массив содержит такой подобный признак завершения. И если бы в данном случае нулевого байта не оказалось бы в строке, то это привело бы к неприятным последствиям. Поэтому обычно применяется другой подход, который заключается в передаче в функцию размера массива.

Третий подход заключается в передаче указателя на конец массива. Можно вручную вычислять указатель на конец массива. А можно использовать встроенные библиотечные функции _std_::_begin_() и _std_::_end_().

Причем _end_ возвращает указатель не на последний элемент, а адрес за последним элементом в массиве.

```cpp
#include <iostream>

using std::cout;
using std::endl;

void test_by_closed_item(const char[]);
void test_by_size(const int[], size_t);
void test_by_range(int*, int*);

int main(int argc, char const *argv[]) {
    const char line[] {"Hello"};
    const int numbers[] {0, 1, 2, 4};

    test_by_closed_item(line);
    test_by_size(numbers, std::size(numbers));
    test_by_range((int*) std::begin(numbers), (int*) std::end(numbers));

    return 0;
}

void test_by_closed_item(const char chars[]) {
    for (size_t i = 0; chars[i] != '\0'; i++) {
        cout << "[test_by_closed_item] " << chars[i]  << endl;
    }
}

void test_by_size(const int numbers[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        cout << "[test_by_size] " << numbers[i] << endl;
    }
}

void test_by_range(int* p_begin, int* p_end) {
    for (int *ptr {p_begin}; ptr != p_end; ptr++) {
        cout << "[test_by_range] " << *ptr << endl;
    }
}
```

```
[test_by_closed_item] H
[test_by_closed_item] e
[test_by_closed_item] l
[test_by_closed_item] l
[test_by_closed_item] o
[test_by_size] 0
[test_by_size] 1
[test_by_size] 2
[test_by_size] 4
[test_by_range] 0
[test_by_range] 1
[test_by_range] 2
[test_by_range] 4
```

#### Константные массивы

Поскольку при передаче массива передается фактически указатель на первый элемент, то используя этот указатель, мы можем изменить элеменnы массива. Если нет необходимости в изменении массива, то лучше параметр-массив определять как константный.

```cpp
#include <iostream>

void print(const int*, const size_t);
void twice(int*, const size_t);

int main(int argc, char const *argv[]) {
    int numbers[] {1, 2, 3, 4, 5};
    size_t size = std::size(numbers);

    print(numbers, size);
    twice(numbers, size);
    print(numbers, size);

    return 0;
}

void print(const int numbers[], const size_t size) {
    for (size_t i {}; i < size; i++) {
        std::cout << "[print] " << numbers[i] << std::endl;
    }
}

void twice(int* numbers, const size_t size) {
    for (size_t i {}; i < size; i++) {
        numbers[i] = 2 * numbers[i];
    }
}
```

```
[print] 1
[print] 2
[print] 3
[print] 4
[print] 5
[print] 2
[print] 4
[print] 6
[print] 8
[print] 10
```

В функции _print_ нет возможности менять значения в _numbers_, т.к. параметр _numbers_ имеет модификатор _const_.

#### Передача массив по ссылке

Еще один сценарий передачи массива в функцию представляет передача массива по ссылке. Прототип функции, которая принимает массив по ссылке, выглядит следующим образом.

Обратите внимание на скобки в записи `(&)`. Они указывают именно на то, что массив передается по ссылке.

```
type func_name(type (&)[]);
```

С одной стороны, может показаться, что в передаче массива по ссылке нет большого смысла, поскольку при передачи массива по значению итак просто передается адрес этого массива. Но с другой стороны, передача массива по ссылке имеет некоторые преимущества. 
- не копируется значение - адрес массива, мы напрямую работаем с оригинальным массивом. 
- передача массива по ссылке позволяет ограничить размер такого массива, соответственно при компиляции компилятор уже будет знать, сколько элементов будет иметь массив.

```cpp
#include <iostream>

void func(const int (&)[5]);

int main(int argc, char const *argv[]) {
    int numbers[] {1, 2, 3, 4, 5};
    func(numbers);

    return 0;
}

void func(const int (&numbers)[5]) {
    for (size_t i {}; i < 5; i++) {
        std::cout << "[func] " << numbers[i] << std::endl;
    }
}
```

```
[func] 1
[func] 2
[func] 3
[func] 4
[func] 5
```

Здесь функция _func_ принимает ссылку строго на массив с 5 элементами. И поскольку мы знаем точный размер массива, то нам нет необходимости передавать в функцию дополнительно размер массива.

Если же мы попробуем передать в функцию массив с другим количеством элементов, то на этапе компиляции мы столкнемся с ошибкой.

#### Передача многомерного массива

Многомерный массив также передается как указатель на его первый элемент. В то же время поскольку элементами многомерного массива являются другие массивы, то указатель на первый элемент многомерного массива фактически будет представлять указатель на массив.

Когда определяется параметр как указатель на массив, размер второй размерности (а также всех последующих размерностей) должен быть определен, так как данный размер является частью типа элемента. 

```
type func_name(type (*arg_name)[some_size]);
```

Здесь предполагается, что передаваемый массив будет двухмерным, и все его подмассивы будут иметь по _some_size_ элемента. Стоит обратить внимание на скобки вокруг имени параметра, которые и позволяют определить параметр как указатель на массив.

```
type func_name(int *arg_name[some_size]);
```

В данном случае параметр определен как массив указателей, а не как указатель на массив.

```cpp
#include <iostream>

void func0(const int (*)[3], const size_t);
void func1(const int [][3], const size_t);

int main(int argc, char const *argv[]) {
    int table [][3] {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    size_t rawsCount {std::size(table)};

    func0(table, rawsCount);
    func1(table, rawsCount);

    return 0;
}

void func0(const int (*rows)[3], const size_t rowsCount) {
    std::cout << "func0" << std::endl;
    size_t columnsCount {std::size(*rows)};
    for (size_t i {}; i < rowsCount; i++) {
        for (size_t j {}; j < columnsCount; j++){
            std::cout << rows[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}

void func1(const int rows [][3], const size_t rowsCount) {
    std::cout << "func1" << std::endl;
    size_t columnsCount {std::size(rows[0])};
    for (size_t i {}; i < rowsCount; i++) {
        for (size_t j {}; j < columnsCount; j++){
            std::cout << rows[i][j] << "\t";
        }
        std::cout << std::endl;
    }
}
```

```
func0
1       2       3
4       5       6
7       8       9
func1
1       2       3
4       5       6
7       8       9
```

---
[Массивы в параметрах функций](https://metanit.com/cpp/tutorial/4.7.php)