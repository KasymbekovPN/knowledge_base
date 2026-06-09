---
tags:
  - programming-language
  - cpp
  - syntax
  - function
  - recursive
---
[[__cpp syntax functions__|<==]]

__Рекурсивные функции__ - это функции, которые вызывают сами себя. Такие функции довольно часто используются для обхода различных представлений. 

И нередко циклические конструкции более эффективны, чем рекурсия. Например, если функция вызывает себя тысячи раз, потребуется большой объем стековой памяти для хранения копий значений аргументов и адреса возврата для каждого вызова, что может привести к тому, что программа быстро исчерпает выделенную для нее память стека, поскольку объем памяти стека обычно фиксирован и ограничен. что может привести к аварийному завершению программы. Поэтому в таких случаях, как правило, лучше использовать альтернативные подходы, например цикл. Однако, несмотря на накладные расходы, использование рекурсии часто может значительно упростить написание программы.

```cpp
#include <iostream>

using std::cout;
using std::endl;

unsigned long long calculate_factorial(unsigned);

int main(int argc, char const *argv[]) {
    unsigned n {5};

    auto result {calculate_factorial(n)};
    cout << n << "! => " << result << endl;

    return 0;
}

unsigned long long calculate_factorial(unsigned n) {
    if (n <= 1) {
        return 1;
    }

    return n * calculate_factorial(n - 1);
}
```

```
5! => 120
```

```cpp
#include <iostream>

using std::cout;
using std::endl;

void sort(int[], size_t, size_t);
void swap(int[], size_t, size_t);

int main(int argc, char const *argv[]) {
    int nums[] {3, 0, 6, -2, -6, 11, 3};
    sort(nums, 0, std::size(nums) - 1);
    for (auto num: nums) {
        cout << num << "\t";
    }
    cout << endl;

    return 0;
}

void sort(int numbers[], size_t start, size_t end) {
    if (start >= end) {
        return;
    }

    size_t current {start};
    for (size_t i {start + 1}; i <= end; i++) {
        if (numbers[i] < numbers[start]) {
            swap(numbers, ++current, i);
        }
    }

    swap(numbers, start, current);

    if (current > start) {
        sort(numbers, start, current - 1);
    }

    if (end > current + 1) {
        sort(numbers, current + 1, end);
    }
}

void swap(int numbers[], size_t first, size_t second) {
    auto temp {numbers[first]};
    numbers[first] = numbers[second];
    numbers[second] = temp;
}
```

---
[Рекурсивная функция](https://metanit.com/cpp/tutorial/3.6.php)