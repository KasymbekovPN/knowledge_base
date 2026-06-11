#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_search(const vector<int>&, const vector<int>&);

int main() {
    vector<int> haystack {1, 2, 3, 4, 5, 6, 3, 4, 5};
    vector<int> needle0 {3, 4, 5};
    vector<int> needle1 {7, 7, 7};

    _test_search(haystack, needle0);
    _test_search(haystack, needle1);

    return 0;
}

void _test_search(const vector<int>& haystach, const vector<int>& needle) {
    auto it = search(
        haystach.begin(),
        haystach.end(),
        needle.begin(),
        needle.end()
    );

    if (it != haystach.end()) {
        cout
            << "Found at position "
            << distance(haystach.begin(), it)
            << endl;
    } else {
        cout << "Not found" << endl;
    }
}
