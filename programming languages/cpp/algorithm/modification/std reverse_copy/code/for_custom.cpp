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
    const vector<Point> SRC = {{1}, {2}, {3}, {4}};

    vector<Point> dst;
    reverse_copy(SRC.begin(), SRC.end(), back_inserter(dst));
    print_vector(dst);

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
