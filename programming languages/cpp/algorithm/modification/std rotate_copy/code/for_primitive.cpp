#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    const vector<int> SRC = {1, 2, 3, 4, 5};

    vector<int> dst;
    rotate_copy(
        SRC.begin(),
        SRC.begin() + 3,
        SRC.end(),
        back_inserter(dst)
    );
    print_vector(dst);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
