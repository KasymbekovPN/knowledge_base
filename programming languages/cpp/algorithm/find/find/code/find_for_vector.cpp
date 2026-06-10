#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_find(const vector<int>&, int);

int main() {
    const vector<int> v {10, 20, 30, 40, 50};
    
    const vector<int> ids {29, 30, 31};
    for (auto& id: ids) {
        _test_find(v, id);
    }
    

    return 0;
}

void _test_find(const vector<int>& v, int id) {
    auto it = std::find(v.begin(), v.end(), id);
    if (it != v.end()) {
        cout << *it << " found" << endl;
    } else {
        cout << id << " not found" << endl;
    }
}
