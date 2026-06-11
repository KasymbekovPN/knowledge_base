#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Point {
    int x;
    int y;
};

void _print_vector(const vector<int>&);

int main() {
    const vector<Point> POINTS {{1, 2}, {3, 4}, {5, 6}};
    
    vector<int> sums(POINTS.size());
    transform(
        POINTS.begin(),
        POINTS.end(),
        sums.begin(),
        [](const Point& point) { return point.x + point.y; }
    );
    _print_vector(sums);

    return 0;
}


void _print_vector(const vector<int>& vec) {
    cout << "{";
    for (auto &&item: vec) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
