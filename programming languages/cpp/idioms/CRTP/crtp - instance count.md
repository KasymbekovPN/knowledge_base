---
tags:
  - programming-language
  - cpp
  - RAII
---
[[programming languages/cpp/idioms/CRTP/_|<=]]

```cpp
#include <iostream>

template<typename T>
struct Countable {
    static inline int count {0};

    Countable() { ++count; }
    ~Countable() { --count; }
};

struct User: Countable<User> {
    std::string name;
    User(const std::string& _name): name{_name} {}
};

struct File: Countable<File> {
    std::string name;
    File(const std::string& _name): name{_name} {}
};

void print_stats();
void test();

int main() {
    print_stats();
    test();
    print_stats();

    return 0;
}

void print_stats() {
    std::cout
        << "User counter: "
        << User::count
        << std::endl;
    std::cout
        << "File counter: "
        << File::count
        << std::endl;
}

void test() {
    User ua {"ua"};
    User ub {"ub"};
    File fa {"fa"};

    print_stats();
} // -> instance's destoying -> decrement of T::count
```

```
User counter: 0
File counter: 0
User counter: 2
File counter: 1
User counter: 0
File counter: 0
```
