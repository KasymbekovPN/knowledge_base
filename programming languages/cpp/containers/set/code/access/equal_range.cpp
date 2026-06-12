#include <iostream>
#include <set>

using std::pair;
using std::set;
using IT = pair<set<int>::iterator, set<int>::iterator>;

void _test_equal_range(const IT&);

int main() {
    set<int> s {1, 2, 3};
    _test_equal_range(s.equal_range(2));
    _test_equal_range(s.equal_range(42));
    return 0;
}

void _test_equal_range(const IT& it_pair) {
    if (it_pair.first != it_pair.second) {
        std::cout << *it_pair.first;
    } else {
        std::cout << "Absence";
    }
    std::cout << std::endl;
}