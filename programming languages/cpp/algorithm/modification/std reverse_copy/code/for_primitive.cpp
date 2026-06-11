#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    const vector<int> SRC {1, 2, 3};
    print_vector(SRC);

    vector<int> dst;
    reverse_copy(SRC.begin(), SRC.end(), back_inserter(dst));
    print_vector(dst);
    
    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (const auto &item : container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
