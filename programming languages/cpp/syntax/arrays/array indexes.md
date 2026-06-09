---
tags:
  - programming-language
  - cpp
  - array
  - index
  - getting
  - setting
---
[[__cpp syntax arrays index__|<==]]

После определения массива мы можем обратиться к его отдельным элементам по индексу. Индексы начинаются с нуля, поэтому для обращения к первому элементу необходимо использовать индекс __0__. 

```cpp
#include <iostream>

using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    const size_t SIZE = 4;
    int nums[SIZE] {100, 101, 102, 103};

    int &first {nums[0]};
    cout << "first <= " << first << endl;

    first++;
    cout << "first after increment <= " << first << endl;

    int seventh = nums[7];
    cout << "seventh <= " << seventh << endl;

    return 0;
}
```

```
first <= 100
first after increment <= 101
seventh <= 0
```

При обращении по индексу следует учитывать, что мы не можем обратиться по несуществующему индексу. Так, если в массиве 4 элемента, то мы можем использовать индексы с 0 до 3 для обращения к его элементам. 

При компиляции `int seventh = nums[7];` будет или ошибка, или предупреждение.
```
warning: array index 7 is past the end of the array (that has type 'int[4]') [-Warray-bounds]
```

---
[Массивы](https://metanit.com/cpp/tutorial/2.15.php)