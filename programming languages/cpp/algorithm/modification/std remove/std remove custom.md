---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std remove/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

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
    vector<Point> points {{1, 2}, {3, 4}, {3, 4}, {5, 6}};
    print_vector(points);

    auto new_end = remove(points.begin(), points.end(), Point(3, 4));
    print_vector(points);

    points.erase(new_end, points.end());
    print_vector(points);

    return 0;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os << "{" << point.x << ", " << point.y << "}" ;
}

void print_vector(const vector<Point>& vec) {
    cout << "{";
    for (auto &&item: vec) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{{1, 2} {3, 4} {3, 4} {5, 6} }
{{1, 2} {5, 6} {3, 4} {5, 6} }
{{1, 2} {5, 6} }
```
