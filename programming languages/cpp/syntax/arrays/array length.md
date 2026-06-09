---
tags:
  - programming-language
  - cpp
  - syntax
  - array
  - length
  - size-of
---
[[__cpp syntax arrays index__|<==]]

Для получения длины массива используется:
- `sizeof()`
- `std::size()`

```cpp
#include <iostream>

using std::size;
using std::cout;
using std::endl;

int main(int argc, char const *argv[]) {
    int nums[4] {1, 2, 3, 4};

    cout << "sizeof(nums) <= " << sizeof(nums) << endl;
    cout << "sizeof(nums[0]) <= " << sizeof(nums[0]) << endl;
    cout << "size(nums) <= " << size(nums) << endl;
    
    return 0;
}
```

```
sizeof(nums) <= 16
sizeof(nums[0]) <= 4
size(nums) <= 4
```

По сути длина массива равна совокупной длине его элементов. Все элементы представляют один и тот же тип и занимают один и тот же размер в памяти. Поэтому с помощью выражения `sizeof(numbers)` находим длину всего массива в байтах, а с помощью выражения `sizeof(numbers[0])` - длину одного элемента в байтах. Разделив два значения, можно получить количество элементов в массиве.

`std::size` вернет количество элементов.

---
[Массивы](https://metanit.com/cpp/tutorial/2.15.php)