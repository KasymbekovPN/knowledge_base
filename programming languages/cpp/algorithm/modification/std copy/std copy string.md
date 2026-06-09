---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/_|<=]]

> ⚠️ `std::copy` не добавляет завершающий ноль (`'\0'`) автоматически!  
Если вы хотите получить null-terminated строку, нужно сделать это вручную:

```cpp
#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

int main() {
    const string SOURCE {"hello, world !!!"};

    char dst[20] = {};
    copy(SOURCE.begin(), SOURCE.end(), dst);
    dst[SOURCE.size()] = '\0';

    cout << "dst: " << dst << endl;

    return 0;
}
```

```
dst: hello, world !!!
```
