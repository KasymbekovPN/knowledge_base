---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/generation/std generate_n/_|<=]]

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

class Functor {
private:
    int _counter;
public:
    Functor(): _counter{0} {}
    int operator()() {
        return _counter--;
    }
};

template<typename T>
void print_vector(const vector<T>&, const string&&);

int main() {
    vector<int> target0;
    int counter {0};
    generate_n(
        back_inserter(target0),
        5,
        [&counter] {return counter++;}
    );
    print_vector(target0, "TARGET0");

    vector<int> target1;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);
    generate_n(
        back_inserter(target1),
        5,
        [&dist, &gen]() {return dist(gen);}
    );
    print_vector(target1, "TARGET1");

    vector<int> target2;
    generate_n(
        back_inserter(target2),
        5,
        Functor()
    );
    print_vector(target2, "TARGET2");

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
```

```
[TARGET0]{0, 1, 2, 3, 4}
[TARGET1]{64, 28, 12, 82, 57}
[TARGET2]{0, -1, -2, -3, -4}
```
