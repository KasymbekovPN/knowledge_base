---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/classification/std any_of/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main(){
    const vector<int> NUMBERS0 {1, 2, 3, 4, 5};
    const vector<int> NUMBERS1 {1, 3, 5, 7, 9};

    auto&& is_even = [](const int _x) {
        return _x % 2 == 0;
    };
    cout
        << "NUMBERS0: "
        << any_of(
            NUMBERS0.begin(),
            NUMBERS0.end(),
            is_even
        )
        << endl;
    cout
        << "NUMBERS1: "
        << any_of(
            NUMBERS1.begin(),
            NUMBERS1.end(),
            is_even
        )
        << endl;

    return 0;
}
```

```
NUMBERS0: 1
NUMBERS1: 0
```
