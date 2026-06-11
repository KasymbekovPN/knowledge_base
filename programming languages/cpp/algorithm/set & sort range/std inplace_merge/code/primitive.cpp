#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> numbers {1, 3, 5, 2, 4, 6};
    print_vector(numbers, "original");

    inplace_merge(numbers.begin(), numbers.begin() + 3, numbers.end());
    print_vector(numbers, "after");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& container, const string&& lbl) {
    cout << "[" << lbl << "] {";
    string delimiter {""};
    for (auto &&item: container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
