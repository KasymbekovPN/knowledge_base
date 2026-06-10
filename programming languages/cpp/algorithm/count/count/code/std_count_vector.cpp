#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main() {
    const vector<int> v {1, 2, 3, 2, 2, 4, 5};
    const int target {2};

    cout
        << "Number of " << target << " => "
        << count(v.begin(), v.end(), target)
        << endl;

    return 0;
}
