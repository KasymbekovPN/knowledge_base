#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

void print_vector(const vector<int>&);

int main() {
    vector<int> numbers {1, 2, 3, 4, 5};
    print_vector(numbers);

    reverse(numbers.begin(), numbers.end());
    print_vector(numbers);

    return 0;
}


void print_vector(const vector<int>& container) {
    cout << "{";
    for (const auto &item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
