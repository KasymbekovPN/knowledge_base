---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/_|<=]]

```cpp
#include <iostream>
#include <algorithm>

using namespace std;

void _print_array(int*, int*);

int main() {
    const int SOURCE[] {1, 2, 3, 4, 5};
    const size_t SIZE {std::size(SOURCE)};

    int dest0[SIZE];
    copy(SOURCE, SOURCE + SIZE, dest0);
    _print_array(begin(dest0), end(dest0));
  
    int dest1[SIZE + 5];
    copy(SOURCE, SOURCE + SIZE, dest1);
    _print_array(begin(dest1), end(dest1));

    return 0;
}

void _print_array(int* b, int* e) {
    cout << "{";
    for (int* p {b}; p != e; p++) {
        cout << *p << " ";
    }
    cout << "}" << endl;
}
```

```
{1 2 3 4 5 }
{1 2 3 4 5 32759 -1902230328 32759 -1902230328 32759 }
```
