#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

struct Value { int x; };

bool operator==(const Value&, const Value&);
ostream& operator<<(ostream&, const Value&);
void _print_vector(const vector<Value>&);

int main() {
    vector<Value> src = {{1}, {2}, {2}, {3}};
    vector<Value> dest;

    auto old_value = Value(2);
    auto new_value = Value(42);
    replace_copy(src.begin(), src.end(), back_inserter(dest), old_value, new_value);
    _print_vector(dest);

    return 0;
}


bool operator==(const Value& a, const Value& b) {
    return a.x == b.x;
}

ostream& operator<<(ostream& os, const Value& p) {
    return os << "{" << p.x << "}";
}

void _print_vector(const vector<Value>& vector)  {
    cout << "{";
    for (auto &i: vector) {
        cout << i << " ";
    }
    cout << "}" << endl;
}
