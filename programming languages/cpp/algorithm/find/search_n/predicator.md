---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/search_n/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Predicator {
    int threshold;

    Predicator(int threshold): threshold(threshold) {}
    bool operator()(int x, int y) {
        return x > threshold && y > threshold;
    }
};

void _test_search_n(const vector<int>&, int, int, const Predicator&);

int main() {
    vector<int> data = {10, 20, 30, 40, 40, 40, 70, 80};
    Predicator pred0 = Predicator(35);
    Predicator pred1 = Predicator(123);

    _test_search_n(data, 3, 40, pred0);
    _test_search_n(data, 3, 40, pred1);

    return 0;
}

void _test_search_n(const vector<int>& data,
					int count,
					int value,
					const Predicator& pred) {
    auto it = search_n(
        data.begin(),
        data.end(),
        count,
        value,
        pred
    );

    if (it != data.end()) {
        cout
            << "Founf sequence starting at "
            << distance(data.begin(), it) << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Founf sequence starting at 3
Not found
```
