---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std rotate_copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

struct Point {
    int x;
};

ostream& operator<<(ostream&, const Point&);
void print_vector(const vector<Point>&);

int main() {
    const vector<Point> POINTS = {{1}, {2}, {3}, {4}, {5}};

    vector<Point> rotated;
    rotate_copy(
        POINTS.begin(),
        POINTS.begin() + 1,
        POINTS.end(),
        back_inserter(rotated)
    );
    print_vector(rotated);

    return 0;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os << "{" << point.x << "}";
}

void print_vector(const vector<Point>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{{2} {3} {4} {5} {1} }
```
