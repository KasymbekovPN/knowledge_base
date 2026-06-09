---
tags:
  - programming-language
  - cpp
  - syntax
  - dynamic
  - array
---
[[__cpp syntax dynamic memory__|<==]]

Кроме отдельных динамических объектов в языке __C++__ мы можем использовать динамические массивы. Для выделения памяти под динамический массив также используется оператор _new_, после которого в квадратных скобках указывается, сколько массив будет содержать объектов.

```cpp
int* first_numbers {new int[SIZE]};
int* second_numbers = new int[SIZE];
```

#### Инициализация

```cpp
#include <iostream>

#include <string>

using std::string;
using std::cout;
using std::endl;
using std::begin;
using std::end;

void print(string, int*, int*);

int main(int argc, char const *argv[]) {
    const int SIZE {4};
    int* numbers0 {new int[SIZE]{}};
    int* numbers1 {new int[SIZE]{1, 2, 3, 4}};
    int* numbers2 {new int[SIZE]{1, 2}};
    int* numbers3 = new int[SIZE]();
    int* numbers4 = new int[SIZE]{};
    int* numbers5 = new int[SIZE]{1, 2, 3, 4};
    int* numbers6 = new int[SIZE]{1, 2};

    print("numbers0", numbers0, numbers0 + SIZE);
    print("numbers1", numbers1, numbers1 + SIZE);
    print("numbers2", numbers2, numbers2 + SIZE);
    print("numbers3", numbers3, numbers3 + SIZE);
    print("numbers4", numbers4, numbers4 + SIZE);
    print("numbers5", numbers5, numbers5 + SIZE);
    print("numbers6", numbers6, numbers6 + SIZE);

    return 0;
}

void print(string label, int* begin, int* end) {
    cout << "[print] " << label;
    for (int* it {begin}; it != end; it++) {
        cout << " " << *it;
    }
    cout << endl;
}
```

```
[print] numbers0 0 0 0 0
[print] numbers1 1 2 3 4
[print] numbers2 1 2 0 0
[print] numbers3 0 0 0 0
[print] numbers4 0 0 0 0
[print] numbers5 1 2 3 4
[print] numbers6 1 2 0 0
```

Здесь оператор _new_ возвращает указатель на объект типа _int_ - первый элемент в созданном массиве.

При инициализации массива конкретными значениями следует учитывать, что если значений в фигурных скобках больше, чем длина массива, то оператор _new_ потерпит неудачу и не сможет создать массив.

Если переданных значений, наоборот, меньше, то элементы, для которых не предоставлены значения, инициализируются значением по умолчанию.

Стоит отметить, что в стандарт _С++20_ добавлена возможность выведения размера массива, поэтому, если применяется стандарт _С++20_, то можно не указывать длину массива.

```cpp
#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;
using std::begin;
using std::end;

void print(string, int*, int*);

int main(int argc, char const *argv[]) {
    const int SIZE {4};
    int* numbers0 {new int[SIZE]{}};
    int* numbers1 {new int[SIZE]{1, 2, 3, 4}};
    int* numbers2 {new int[SIZE]{1, 2}};
    int* numbers3 {new int[]{1, 2}};

    print("numbers0", numbers0, numbers0 + SIZE);
    print("numbers1", numbers1, numbers1 + SIZE);
    print("numbers2", numbers2, numbers2 + SIZE);
    print("numbers3", numbers3, numbers3 + 2);

    return 0;
}

void print(string label, int* begin, int* end) {
    cout << "[print] " << label << " {";
    for (int* it {begin}; it != end; it++) {
        cout << " " << *it;
    }
    cout << "}" << endl;
}
```

```
[print] numbers0 { 0 0 0 0}
[print] numbers1 { 1 2 3 4}
[print] numbers2 { 1 2 0 0}
[print] numbers3 { 1 2}
```

Для задания размера динамического массива мы можем применять `обычную` переменную, а `не константу`, как в случае со стандартными массивами

#### Удаление

Для удаления динамического массива и освобождения его памяти применяется специальная форма оператора _delete_.

```cpp
delete [] ptr_to_dyn_array;
ptr_to_dyn_array = nullptr;
```

#### Многомерные массивы

Многомерный массив это массив массивов.  В общем случае это выглядит так.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    size_t rows {3};
    size_t columns {2};
    int** numbers {new int*[rows]{}}; // <==

    for (size_t i{}; i < rows; i++) {
        numbers[i] = new int[columns]{};
    }

    for (size_t i {}; i < rows; i++) {
        for (size_t j {}; j < columns; j++) {
            cout << numbers[i][j] << " ";
        }
        cout << "\n";
    }

    for (size_t i {}; i < rows; i++) {
        delete[] numbers[i];
    }
    delete[] numbers;

    return 0;
}
```

```
0 0 
0 0
0 0
```

#### Указатель на массив

От типа _int**_, который представляет __указатель на указатель__ (`pointer-to-pointer`) следует отличать ситуацию __указатель на массив__ (`pointer to array`). 

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    size_t n{3};
    int (*ptr)[2] = new int[n][2];
    int k{};

    for (size_t i {}; i < n; i++) {
        for (size_t j {}; j < 2; j++) {
            ptr[i][j] = ++k;
        }
    }

    for (size_t i {}; i < n; i++) {
        for (size_t j {}; j < 2; j++) {
            cout << ptr[i][j] << " ";
        }
        cout << endl;
    }

    delete[] ptr;
    ptr = nullptr;

    return 0;
}
```

```
1 2 
3 4
5 6
```

---
[Динамические массивы](https://metanit.com/cpp/tutorial/4.12.php)