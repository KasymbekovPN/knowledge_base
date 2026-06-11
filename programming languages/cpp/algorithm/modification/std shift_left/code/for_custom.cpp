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
    vector<Point> points {{1}, {2}, {3}, {4}, {5}};
    print_vector(points);

    auto new_end = shift_left(points.begin(), points.end(), 1);
    print_vector(points);

    points.erase(new_end, points.end());
    print_vector(points);
    
    return 0;
}

ostream& operator<<(ostream& os, const Point& point) {
    return os << "{" << point.x << "}";
}

void print_vector(const vector<Point>& points) {
    cout << "{";
    for (auto &&item: points) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
