#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> target(5);
    fill(target.begin(), target.end(), 42);
    print_vector(target, "TARGET");

    return 0;
}


template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "]{";
    string delimiter {""};
    for (auto &&item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
