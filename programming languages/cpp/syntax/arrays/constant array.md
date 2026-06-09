---
tags:
  - programming-language
  - cpp
  - syntax
  - array
  - constants
---
[[__cpp syntax arrays index__|<==]]

Если необходимо, чтобы нельзя было изменять значения элементов массива, то такой массив можно определить как константный с помощью ключевого слова _const_

```cpp
#include <iostream>

int main(int argc, char const *argv[]) {
    const int nums[4]{0, 1, 2, 3};
    nums[1] = 111; // Error

    return 0;
}
```

```
error: cannot assign to variable 'nums' with const-qualified type 'const int[4]'
```

---
[Массивы](https://metanit.com/cpp/tutorial/2.15.php)