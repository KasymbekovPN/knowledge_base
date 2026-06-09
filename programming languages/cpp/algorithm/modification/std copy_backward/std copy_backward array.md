---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std copy_backward/_|<=]]

```cpp
#include <iostream>
#include <algorithm>

using namespace std;

void _print_array(int*, int*);

int main() {
    int src[] = {1, 2, 3, 4, 5, 6, 7, 8};
    size_t size = sizeof(src) / sizeof(src[0]);

    int dst[10] = {};
    copy_backward(src, src + size, dst + 10);
    _print_array(begin(dst), end(dst));

    copy_backward(src, src + size - 3, src + size);
    _print_array(begin(src), end(src));

    return 0;
}

void _print_array(int* p_begin, int* p_end) {
    cout << "{ ";
    for (int* p {p_begin}; p != p_end; p++) {
        cout << *p << " ";
    }
    cout << "}" << endl;
}
```

```
{ 0 0 1 2 3 4 5 6 7 8 }
{ 1 2 3 1 2 3 4 5 }
```

