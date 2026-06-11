#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

int main(int argc, char const *argv[]) {
    vector<int> haystack = {1, 2, 3, 4, 5, 6, 3, 4, 5};
    vector<int> needle = {3, 4, 5};

    auto it = find_end(haystack.begin(), haystack.end(),
                             needle.begin(), needle.end());

    if (it != haystack.end()) {
        cout << "Found sequence at position: "
             << distance(haystack.begin(), it) << endl;;
    } else {
        cout << "Sequence not found\n";
    }
}
