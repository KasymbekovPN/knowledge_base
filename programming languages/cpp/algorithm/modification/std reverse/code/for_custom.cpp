#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Point {
    int x, y;
};

void print_vector(const vector<Point>&);
ostream& operator<<(ostream&, const Point&);

int main() {
    vector<Point> points {
        {1, 2},
        {3, 4},
        {5, 6}
    };
    print_vector(points);

    reverse(points.begin(), points.end());
    print_vector(points);

    return 0;
}

void print_vector(const vector<Point>& points) {
    cout << "{";
    for (const auto &point: points) {
        cout << point << " ";
    }
    cout << "}" << endl;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os
        << "{" << point.x << ", "
        << point.y << "}" << endl;
}
