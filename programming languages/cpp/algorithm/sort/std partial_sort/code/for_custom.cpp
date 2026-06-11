#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Value {
    int x;

    bool operator<(const Value& other) const {
        return x < other.x;
    }
};

ostream& operator<<(ostream&, const Value&);
void print_vector(const vector<Value>&);

int main(int argc, char const *argv[]) {
    vector<Value> numbers {{5}, {2}, {8}, {1}, {9}, {3}, {7}};
    print_vector(numbers);

    partial_sort(numbers.begin(), numbers.begin() + 4, numbers.end());
    print_vector(numbers);

    partial_sort(
        numbers.begin(),
        numbers.begin() + 4,
        numbers.end(),
        [](const Value& v0, const Value& v1) { return v0.x > v1.x; });
    print_vector(numbers);

    return 0;
}

ostream& operator<<(ostream& os, const Value& value) {
    return os << "{" << value.x << "}";
}

void print_vector(const vector<Value>& container) {
    cout << "{";
    for (auto &&item: container) {
        cout << item << " ";
    }
    cout << "}" << endl;
}
