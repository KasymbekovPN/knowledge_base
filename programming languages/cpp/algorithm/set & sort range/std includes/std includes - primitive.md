---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/set & sort range/std includes/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {1, 2, 3, 4, 5, 6, 7};
    vector<int> part0 {2, 3, 4};
    vector<int> part1 {2, 4, 3};
    vector<int> part2 {5, 3};

    cout << boolalpha;
    cout << includes(
        numbers.begin(),
        numbers.end(),
        part0.begin(),
        part0.end()
    ) << endl;

    cout << includes(
        numbers.begin(),
        numbers.end(),
        part1.begin(),
        part1.end()
    ) << endl;

    cout << includes(
        numbers.begin(),
        numbers.end(),
        part2.begin(),
        part2.end(),
        greater<int>()
    ) << endl;
    cout << noboolalpha;

    return 0;
}
```

```
true
false
false
```
