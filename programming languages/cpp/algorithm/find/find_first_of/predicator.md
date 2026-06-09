---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/find_first_of/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

bool _close_to(int, int);
void _test_find_first_of(const vector<int>&,
						 const vector<int>&,
						 bool(*)(int, int));

int main(int argc, char const *argv[]) {
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::vector<int> target0 = {28, 39, 52};
    std::vector<int> target1 = {128, 139, 152};

    _test_find_first_of(data, target0, _close_to);
    _test_find_first_of(data, target1, _close_to);

    return 0;
}

bool _close_to(int x, int y) {
    return abs(x - y) <= 2;

}

void _test_find_first_of(const vector<int>& data,
                         const vector<int>& target,
                         bool(*pred)(int, int)) {
    auto it = find_first_of(
        data.begin(),
        data.end(),
        target.begin(),
        target.end(),
        pred
    );

    if (it != data.end()) {
        cout << "Found: "  << *it << endl;
    } else {
        cout << "No values close to target" << endl;
    }
}
```

```
Found: 30
No values close to target
```
