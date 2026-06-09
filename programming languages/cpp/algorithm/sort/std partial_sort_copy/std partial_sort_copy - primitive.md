---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/sort/std partial_sort_copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main(int argc, char const *argv[]) {
    const vector<int> SRC {5, 2, 8, 1, 9, 3, 7};
    const size_t SIZE {4};

    vector<int> numbers(SIZE);
    print_vector(numbers);

    partial_sort_copy(
	    SRC.begin(),
	    SRC.end(),
	    numbers.begin(),
	    numbers.end()
	);
    print_vector(numbers);

    partial_sort_copy(
        SRC.begin(),
        SRC.end(),
        numbers.begin(),
        numbers.end(),
        [](int x, int y) { return x > y; });
    print_vector(numbers);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{0 0 0 0 }
{1 2 3 5 }
{9 8 7 5 }
```
