#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

struct Value {
    int x;
    bool operator==(const Value& other) const {
        return x == other.x;
    }
};

ostream& operator<<(ostream& os, const Value& p) {
    return os << "{" << p.x << "}";
}

void _print_vector(const vector<Value>&);

int main() {
    vector<Value> values {{1}, {2}, {3}};

    replace(values.begin(), values.end(), Value(2), Value(42));
    _print_vector(values);

    return 0;
}


void _print_vector(const vector<Value>& v) {
    cout << "{ ";
    for (const auto &i: v) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
