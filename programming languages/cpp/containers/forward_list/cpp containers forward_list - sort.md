---
tags:
  - programming-language
  - cpp
  - syntax
  - container
  - forward_list
---
[[_cpp containers - forward_list|<=]]

#### Сортировка без компаратора

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(); // ascending
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

#### Сортировка `std::forward_list` со стандартным компаратором

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(std::less<int>()); // ascending
    print_flist(numbers);

    numbers.sort(std::greater<int>()); // descending
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
flist => 123 17 200 1 -1 
flist => -1 1 17 123 200
flist => 200 123 17 1 -1
```

#### Сортировка с пользовательской функцией-компаратором

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

bool compare_descending(int, int);
bool compare_ascending(int, int);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(compare_ascending);
    print_flist(numbers);

    numbers.sort(compare_descending);
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}

bool compare_descending(int a, int b) {
    return a > b;
}

bool compare_ascending(int a, int b) {
    return b > a;
}
```

```
flist => 123 17 200 1 -1 
flist => -1 1 17 123 200
flist => 200 123 17 1 -1
```

#### Использование лямбда-функции

```cpp
#include <iostream>
#include <forward_list>

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort([](int a, int b){return a > b;});
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

#### Использование функционального объекта

```cpp
#include <iostream>
#include <forward_list>

struct CompareDescending {
    bool operator()(int a, int b) {
        return a > b;
    }
};

template<typename T>
void print_flist(const std::forward_list<T>&);

int main() {
    std::forward_list<int> numbers {123, 17, 200, 1, -1};
    print_flist(numbers);

    numbers.sort(CompareDescending());
    print_flist(numbers);

    return 0;
}

template<typename T>
void print_flist(const std::forward_list<T>& flist) {
    std::cout << "flist => ";
    for (const auto &item: flist) {
        std::cout << item << " ";
    }
    std::cout << std::endl;
}
```

```
flist => 123 17 200 1 -1 
flist => 200 123 17 1 -1
```
