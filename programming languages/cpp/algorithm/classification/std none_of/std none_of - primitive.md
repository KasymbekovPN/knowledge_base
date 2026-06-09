---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/classification/std none_of/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> NUMBERS0 {1, 2, 3, 4};
    const vector<int> NUMBERS1 {2, 4, 6, 8};

    auto&& pred = [](const int _x){
        return _x % 2 == 1;
    };
    cout
        << "0) "
        << none_of(NUMBERS0.begin(), NUMBERS0.end(), pred)
        << endl;
    cout
        << "1) "
        << none_of(NUMBERS1.begin(), NUMBERS1.end(), pred)
        << endl;

    return 0;
}
```

```
0) 0
1) 1
```
