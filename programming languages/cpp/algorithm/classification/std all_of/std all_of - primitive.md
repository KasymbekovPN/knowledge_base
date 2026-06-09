---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/classification/std all_of/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> NUMBERS0 {1, 3, 5, 7};
    const vector<int> NUMBERS1 {2, 4, 6, 8};

    auto&& pred = [](const int _x){
        return _x % 2 == 1;
    };
    cout
        << "0) "
        << all_of(NUMBERS0.begin(), NUMBERS0.end(), pred)
        << endl;
    cout
        << "1) "
        << all_of(NUMBERS1.begin(), NUMBERS1.end(), pred)
        << endl;

    return 0;
}
```

```
0) 1
1) 0
```
