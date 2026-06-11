#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Point {
    int x;
};

ostream& operator<<(ostream&, const Point&);
void print_vector(const vector<Point>&);

int main() {
    vector<Point> points = {{1}, {2}, {3}, {4}, {5}};

    rotate(points. begin(), points.begin() + 2, points.end());
    print_vector(points);

    return 0;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os << "{" << point.x << "}";
}

void print_vector(const vector<Point>& container) {
    cout << "{";
    for (auto &&item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
