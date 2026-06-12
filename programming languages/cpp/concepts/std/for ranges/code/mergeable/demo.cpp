#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

struct Value {
    int x{};

    Value(int _x): x{_x} {}

    bool operator==(const Value&) const = default;
    auto operator<=>(const Value&) const = default;
};

std::ostream& operator<<(std::ostream& _os, const Value& _value) {
    return _os << "{" << _value.x << "}";
}

template<typename T>
requires std::mergeable<
    typename std::vector<T>::iterator,
    typename std::vector<T>::iterator,
    std::back_insert_iterator<std::vector<T>>
>
void test(std::vector<T>& _in0,
          std::vector<T>& _in1,
          std::vector<T>& _out) {
    std::ranges::merge(
        _in0,
        _in1,
        std::back_inserter(_out)
    );
}

template<typename T>
void print(std::vector<T>& _vec) {
    for (auto& item: _vec) {
        std::cout << item << " ";
    }
    std::cout << std::endl; 
}

int main() {
    std::vector<int> vin0 {1, 2, 3};
    std::vector<int> vin1 {3, 4, 5};
    std::vector<int> vout;
    test(vin0, vin1, vout);
    print(vout);

    std::vector<Value> vain0 {{0}, {1}, {2}};
    std::vector<Value> vain1 {{2}, {3}, {4}};
    std::vector<Value> vaout;
    test(vain0, vain1, vaout);
    print(vaout);

    return 0;
}
