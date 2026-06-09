---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/find/search/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_search(const vector<int>&, const vector<int>&);

int main() {
    vector<int> haystack {1, 2, 3, 4, 5, 6, 3, 4, 5};
    vector<int> needle0 {3, 4, 5};
    vector<int> needle1 {7, 7, 7};

    _test_search(haystack, needle0);
    _test_search(haystack, needle1);

    return 0;
}

void _test_search(const vector<int>& haystach, const vector<int>& needle) {
    auto it = search(
        haystach.begin(),
        haystach.end(),
        needle.begin(),
        needle.end()
    );

    if (it != haystach.end()) {
        cout
            << "Found at position "
            << distance(haystach.begin(), it)
            << endl;
    } else {
        cout << "Not found" << endl;
    }
}
```

```
Found at position 2
Not found
```


---

---

## ✅ Пример 3: с пользовательским предикатом (C++11+)

```cpp
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype> // для std::tolower

bool case_insensitive_equal(char a, char b) {
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

int main() {
    std::string text = "Hello world! HELLO universe!";
    std::string pattern = "HELLO";

    auto it = std::search(text.begin(), text.end(), pattern.begin(), pattern.end(), case_insensitive_equal);

    if (it != text.end()) {
        std::cout << "Pattern found at index: "
                  << std::distance(text.begin(), it) << "\n";
        std::cout << "Matched substring: " << std::string(it, it + pattern.size()) << "\n";
    } else {
        std::cout << "Pattern not found\n";
    }
}
```

### Вывод:
```
Pattern found at index: 0
Matched substring: Hello
```

---

