---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/modification/std unique copy/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include<iterator>

using namespace std;

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

ostream& operator<<(ostream&, const Point&);
void print_vector(const vector<Point>&);

int main() {
    const vector<Point> SRC {
        {1, 2},
        {1, 2},
        {1, 3},
        {3, 4},
        {3, 4},
        {3, 5}
    };
    print_vector(SRC);

    vector<Point> dst0;
    unique_copy(SRC.begin(), SRC.end(), back_inserter(dst0));
    print_vector(dst0);

    vector<Point> dst1;
    unique_copy(
        SRC.begin(),
        SRC.end(),
        back_inserter(dst1),
        [](const Point& a, const Point& b) {return a.x == b.x;}
    );
    print_vector(dst1);

    return 0;
}

ostream& operator<<(ostream& os, const Point& p) {
    return os
        << "{x: "<< p.x
        << ", y: " << p.y
        << "}";
}

void print_vector(const vector<Point> &container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
```

```
{{x: 1, y: 2} {x: 1, y: 2} {x: 1, y: 3} {x: 3, y: 4} {x: 3, y: 4} {x: 3, y: 5} }
{{x: 1, y: 2} {x: 1, y: 3} {x: 3, y: 4} {x: 3, y: 5} }
{{x: 1, y: 2} {x: 3, y: 4} }
```

