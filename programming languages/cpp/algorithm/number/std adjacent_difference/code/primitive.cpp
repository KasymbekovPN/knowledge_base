#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    const vector<int> NUMBERS {2, 4, 8, 16, 32};
    print_vector(NUMBERS, "NUMBERS");

    vector<int> result_diff;
    adjacent_difference(
        NUMBERS.begin(),
        NUMBERS.end(),
        back_inserter(result_diff)
    );
    print_vector(result_diff, "result_diff");

    vector<int> result_div;
    adjacent_difference(
        NUMBERS.begin(),
        NUMBERS.end(),
        back_inserter(result_div),
        [](const int _a, const int _b) {
            return _a / _b;
        }
    );
    print_vector(result_div, "result_div");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "]{";
    string delimiter {""};
    for (auto &item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
