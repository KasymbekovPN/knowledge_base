#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> vec {1, 1, 2, 3, 4, 4, 42};
    print_vector(vec);

    auto new_end = unique(vec.begin(), vec.end());
    print_vector(vec);

    vec.erase(new_end, vec.end());
    print_vector(vec);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
