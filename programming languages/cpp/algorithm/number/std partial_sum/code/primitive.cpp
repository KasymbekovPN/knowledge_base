#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    const vector<int> NUMBERS {1, 2, 3, 4, 5};
    const vector<string> WORDS {"A", "B", "C"};
    print_vector(NUMBERS, "NUMBERS");
    print_vector(WORDS, "WORDS");

    vector<int> result_sum;
    partial_sum(
        NUMBERS.begin(),
        NUMBERS.end(),
        back_inserter(result_sum)
    );
    print_vector(result_sum, "result_sum");

    vector<int> result_mul;
    partial_sum(
        NUMBERS.begin(),
        NUMBERS.end(),
        back_inserter(result_mul),
        multiplies<int>()
    );
    print_vector(result_mul, "result_mul");

    vector<string> result_cc;
    partial_sum(
        WORDS.begin(),
        WORDS.end(),
        back_inserter(result_cc),
        [](const string& _a, const string& _b) {
            return _a + "-" + _b;
        }
    );
    print_vector(result_cc, "result_cc");

    return 0;
}

template<typename T>
void print_vector(const vector<T>& _container, const string&& _lbl) {
    cout << "[" << _lbl << "]{";
    string delimiter{""};
    for (auto &item: _container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
