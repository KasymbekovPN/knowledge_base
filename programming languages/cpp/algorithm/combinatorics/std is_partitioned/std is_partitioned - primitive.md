---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/combinatorics/std is_partitioned/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void test(const vector<int>&&, const string&&);

int main() {
    test({2, 4, 6, 1, 3, 5}, "YES");
    test({2, 17, 6, 1, 3, 5}, "NO");

    return 0;
}

void test(const vector<int>&& _container, const string&& _lbl) {
    bool result = is_partitioned(
        _container.begin(),
        _container.end(),
        [](int _x) { return _x % 2 == 0; }
    );
    cout
        << _lbl << " " << boolalpha
        << result << noboolalpha
        << endl;
}
```

```
YES true
NO false
```
