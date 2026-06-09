---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std reverse/_|<=]]

```cpp
#include <iostream>
#include <algorithm>

using namespace std;

int main() {
    string line {"Hello, world !!!"};
    cout << line << endl;

    reverse(line.begin(), line.end());
    cout << line << endl;

    return 0;
}
```

```
Hello, world !!!
!!! dlrow ,olleH
```
