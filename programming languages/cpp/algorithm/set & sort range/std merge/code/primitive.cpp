#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

template<typename T>
void print_vector(const vector<T>&, string&&);

int main() {
    const vector<int> FIRST {1, 42, -2, 100};
    print_vector(FIRST, "FIRST");

    const vector<int> SECOND {-1, 142, 20, 10};
    print_vector(SECOND, "SECOND");

    vector<int> result0;
    merge(
        FIRST.begin(),
        FIRST.end(),
        SECOND.begin(),
        SECOND.end(),
        back_inserter(result0)
    );
    print_vector(result0, "RESULT0");

    vector<int> result1;
    merge(
        FIRST.begin(),
        FIRST.end(),
        SECOND.begin(),
        SECOND.end(),
        back_inserter(result1),
        greater<int>()
    );
    print_vector(result1, "RESULT1");
    
    return 0;
}

template<typename T>
void print_vector(const vector<T>& container, string&& lbl) {
    cout << "[" << lbl << "] {";
    string delimiter {""};
    for (auto &&item: container) {
        cout << delimiter << item;
        delimiter = ", ";
    }
    cout << "}" << endl;
}
