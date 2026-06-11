#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void _test_find_first_of(const vector<int>&, const vector<int>&);

int main() {
    const vector<int> data = {10, 20, 30, 40, 50};
    const vector<int> targets = {35, 40, 45};

    _test_find_first_of(data, targets);

    return 0;
}


void _test_find_first_of(const vector<int>& data, const vector<int>& target) {
    auto it = find_first_of(
        data.begin(),
        data.end(),
        target.begin(),
        target.end()
    );

    if (it != data.end()) {
        cout << "Founf match: " << *it << endl;
    } else {
        cout << "Not found" << endl;
    }
}
