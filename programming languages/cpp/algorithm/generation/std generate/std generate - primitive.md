---
tags:
  - programming-language
  - cpp
  - algorithm
---
[[programming languages/cpp/algorithm/generation/std generate/_|<=]]

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
    vector<int> target0(5);
    int counter {0};
    generate(
        target0.begin(),
        target0.end(),
        [&counter] {return counter++;}
    );
    print_vector(target0, "TARGET0");

    vector<int> target1(5);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(1, 100);
    generate(
        target1.begin(),
        target1.end(),
        [&dist, &gen]() {return dist(gen);}
    );
    print_vector(target1, "TARGET1");

    vector<int> target2(5);
    generate(
        target2.begin(),
        target2.end(),
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
[TARGET1]{52, 62, 98, 35, 59}
[TARGET2]{0, -1, -2, -3, -4}
```
