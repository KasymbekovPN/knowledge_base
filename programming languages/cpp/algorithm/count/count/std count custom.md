---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/count/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

int main() {
    const vector<Point> POINTS {{1, 2}, {3, 4}, {3, 4}, {5, 6}};
    const Point TARGET = {3, 4};

    cout << "Count: " << count(POINTS.begin(), POINTS.end(), TARGET) << endl;

    return 0;
}
```

```
Count: 2
```
