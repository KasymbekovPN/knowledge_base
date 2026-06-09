---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/abjacent_find/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
  
using namespace std;

int main() {
    const vector<int> v {15, 15, 15, 20, 20, 25};
    auto it = adjacent_find(v.begin(), v.end(), [](int a, int b){
        return b > a;
    });

    if (it != v.end()) {
        cout
            << "Found at " << distance(v.begin(), it)
            << ": " << *it << ", " << *(it + 1) << endl;
    } else {
        cout << "Not found" << endl;
    }
    return 0;
}
```

```
Found at 2: 15, 20
```

