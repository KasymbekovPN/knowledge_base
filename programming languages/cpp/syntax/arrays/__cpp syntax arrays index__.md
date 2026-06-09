---
tags:
  - programming-language
  - cpp
  - syntax
  - array
---
[[_cpp syntax|<==]]

__Массив__ представляет набор однотипных данных. Формальное определение массива выглядит следующим образом.

```
type array_name[array_length];
```

```cpp
int numbers[4];
```

Число элементов массива также можно определять через константу
```cpp
const int n = 4;
int numbers[n];
```

Некоторые компиляторы (например, `G++`) также поддерживают установку размера с помощью переменных.

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const size_t size = 4;

    int undef_numbers[size];
    for (size_t i = 0; i < size; i++) {
        cout
	        << "undef_numbers[" << i << "] <= "
	        << undef_numbers[i] << endl;
    }
    cout << endl;
   
    int zero_numbers[size] {};
    for (size_t i = 0; i < size; i++) {
        cout
	        << "zero_numbers[" << i << "] <= "
	        << zero_numbers[i] << endl;
    }
    cout << endl;
    int set_numbers[size] {0, 1, 2, 3};
    for (size_t i = 0; i < size; i++) {
        cout
	        << "set_numbers[" << i << "] <= "
	        << set_numbers[i] << endl;
    }
    cout << endl;

    int not_completed_set_numbers[size] {100, 101};
    for (size_t i = 0; i < size; i++) {
        cout
	        << "not_completed_set_numbers[" << i << "] <= "
	        << not_completed_set_numbers[i] << endl;
    }
    
    // int bad_size_array[size] {1, 2, 3, 4, 5}; // Error

	int nums[] {1, 2, 3};
	// int nums_copy = nums;

    return 0;
}
```

```
undef_numbers[0] <= -37747944
undef_numbers[1] <= 32758
undef_numbers[2] <= -37905183
undef_numbers[3] <= 32758

zero_numbers[0] <= 0
zero_numbers[1] <= 0
zero_numbers[2] <= 0
zero_numbers[3] <= 0

set_numbers[0] <= 0
set_numbers[1] <= 1
set_numbers[2] <= 2
set_numbers[3] <= 3

not_completed_set_numbers[0] <= 100
not_completed_set_numbers[1] <= 101
not_completed_set_numbers[2] <= 0
not_completed_set_numbers[3] <= 0
```

Массив _undef_numbers_ имеет четыре числа, но все эти числа имеют неопределенное значение.

Чтобы установить значения элементов массива, указываются фигурные скобки (инициализатор), внутри которых перечисляются значения для элементов массива.

В случае _zero_numbers_ инициализация производится дефолтными значениями.

В случае _set_numbers_ инициализация производится значениями из инициализатора.

В случае _not_completed_set_numbers_ инициализация первых двух элементов производится значениями из инициализатора, прочие - дефолтные.

Если инициализатор содержит больше элементов, чем массив может принять, то это вызовет ошибку.
```
error: excess elements in array initializer
```

Если размер массива не указан явно, то он выводится из количества переданных значений
```cpp
int nums[] {1, 2, 3}; // nums имеет размер 3
```

При этом не допускается присвоение одному массиву другого массива
```
error: cannot initialize a variable of type 'int' with an lvalue of type 'int[3]'
```

- [[array indexes]]
- [[constant array]]
- [[array length]]
- [[multidimensional arrays]]
- [[symbol array]]

---
[Массивы](https://metanit.com/cpp/tutorial/2.15.php)
