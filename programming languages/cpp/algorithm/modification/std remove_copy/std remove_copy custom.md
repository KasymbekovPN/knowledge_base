---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std remove_copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

struct Point {
    int x;
    int y;
  
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

ostream& operator<<(ostream&, const Point&);
void print_vector(const vector<Point>&);

int main() {
    const vector<Point> POINTS {{1, 2}, {3, 4}, {3, 4}, {5, 6}};
    print_vector(POINTS);

    vector<Point> dst;
    remove_copy(
        POINTS.begin(),
        POINTS.end(),
        back_inserter(dst),
        Point(3, 4)
    );
    print_vector(dst);

    return 0;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os << "{" << point.x << ", " << point.y << "}";
}

void print_vector(const vector<Point>& points) {
    cout << "{";
    for (auto &&point: points) {
        cout << point << " ";
    }
    cout << "}" << endl;
}
```

```
{{1, 2} {3, 4} {3, 4} {5, 6} }
{{1, 2} {5, 6} }
```
