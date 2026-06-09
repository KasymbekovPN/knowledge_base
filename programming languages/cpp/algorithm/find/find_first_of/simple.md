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

using namespace std;

void _test_find_first_of(const vector<int>&, const vector<int>&);

int main() {
    const vector<int> data = {10, 20, 30, 40, 50};
    const vector<int> targets = {35, 40, 45};

    _test_find_first_of(data, targets);

    return 0;
}

void _test_find_first_of(const vector<int>& data,
						 const vector<int>& target) {
    auto it = find_first_of(
        data.begin(),
        data.end(),
        target.begin(),
        target.end()
    );

    if (it != data.end()) {
        cout << "Founf match: " << *it << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Founf match: 40
```

---

---

## ✅ Пример 2: с пользовательским предикатом

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath> // для std::abs

bool close_to(int a, int b) {
    return std::abs(a - b) <= 2;
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::vector<int> targets = {28, 39, 52};

    auto it = std::find_first_of(data.begin(), data.end(),
                                  targets.begin(), targets.end(),
                                  close_to);

    if (it != data.end()) {
        std::cout << "Found value close to target: " << *it << "\n";
    } else {
        std::cout << "No values close to target\n";
    }
}
```

### Вывод:
```
Found value close to target: 30
```

