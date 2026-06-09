---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/combinatorics/std prev_permutation/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    vector<int> numbers {3, 2, 1};
    do {
        cout << "0 ) ";
        for (auto& item: numbers) {
            cout << item << " ";
        }
        cout << endl;
    } while (prev_permutation(numbers.begin(), numbers.end()));

    reverse(numbers.begin(), numbers.end());    
    do {
        cout << "1 ) ";
        for (auto& item: numbers) {
            cout << item << " ";
        }
        cout << endl;
    } while (
	    prev_permutation(numbers.begin(), numbers.end(), greater<int>())
	);

    return 0;
}
```

```
0 ) 3 2 1 
0 ) 3 1 2
0 ) 2 3 1
0 ) 2 1 3
0 ) 1 3 2
0 ) 1 2 3
1 ) 1 2 3
1 ) 1 3 2
1 ) 2 1 3
1 ) 2 3 1
1 ) 3 1 2
1 ) 3 2 1
```
