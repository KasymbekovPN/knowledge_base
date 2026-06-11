#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    const vector<int> SRC {1, 2, 3, 4, 5, 6};
    print_vector(SRC);

    vector<int> dst;
    remove_copy_if(
        SRC.begin(),
        SRC.end(),
        back_inserter(dst),
        [](int x) { return x % 2 == 0; }
    );
    print_vector(dst);

    return 0;
}

void print_vector(const vector<int>& vec) {
    cout << "{";
    for (auto &&item: vec) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
