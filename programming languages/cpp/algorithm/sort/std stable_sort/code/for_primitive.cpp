#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> numbers = {3, 1, 4, 1, 5};
    print_vector(numbers);

    stable_sort(numbers.begin(), numbers.end());
    print_vector(numbers);

    return 0;
}

void print_vector(const vector<int>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item <<  " ";
    }
    cout << "}" << endl;
}
