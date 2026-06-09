---
tags:
  - programming-language
  - cpp
  - container
  - map
---
[[_cpp containers map - init|<=]]

```cpp
#include <iostream>
#include <map>

using std::cout;
using std::endl;
using std::map;
using std::string;

struct ComparatorFunctor {
    bool operator()(const string& lhs, const string& rhs) const {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(),
            rhs.begin(), rhs.end(),
            [](char ch1, char ch2) {return tolower(ch1) < tolower(ch2);}
        );
    }
};

bool compare(const string&, const string&);

template<typename T>
void _print_map(map<string, int, T>&, string);

int main() {
    map<string, int, std::greater<>> map_gr = {
        {"apple", 5},
        {"banana", 3},
        {"cherry", 8}
    };
    _print_map(map_gr, "greater");

    map<string, int, ComparatorFunctor> map_fctr = {
        {"apple", 5},
        {"banana", 3},
        {"cherry", 8}
    };
    _print_map(map_fctr, "functor");

    map<string, int, decltype(&compare)> map_func(compare);
    map_func.insert({"apple", 5});
    map_func.insert({"banana", 3});
    map_func.insert({"cherry", 8});
    _print_map(map_func, "func");

    auto lambda_cmp = [](const std::string& lhs, const std::string& rhs) {
        return lhs > rhs;
    };
    map<string, int, decltype(lambda_cmp)> map_l(lambda_cmp);
    map_l.insert({"apple", 5});
    map_l.insert({"banana", 3});
    map_l.insert({"cherry", 8});
    _print_map(map_l, "lambda");

    cout << "Done!" << endl;

    return 0;
}

template<typename T>
void _print_map(map<string, int, T>& m, string label) {
    cout << "### " << label << " ###" << endl;
    for (auto &pair: m) {
        cout << "{" << pair.first
            << ", " << pair.second
            << "}" << endl;
    }
}

bool compare(const string& lhs, const string& rhs) {
    return lhs.length() > rhs.length();
}
```
```
### greater ###
{cherry, 8}
{banana, 3}
{apple, 5}
### functor ###
{apple, 5}
{banana, 3}
{cherry, 8}
### func ###
{banana, 3}
{apple, 5}
### lambda ###
{cherry, 8}
{banana, 3}
{apple, 5}
Done!
```