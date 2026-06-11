#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> vec {10, 20, 30, 40, 50};
    print_vector(vec);
  
    auto new_end = shift_right(vec.begin(), vec.end(), 2);
    print_vector(vec);

    vec.erase(vec.begin(), new_end);
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
