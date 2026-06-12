#include <iostream>
#include <cstring>

class BadImpl {
private:
    char* data;
public:
    BadImpl(const char* _str) {
        data = new char[strlen(_str) + 1];
        strcpy(data, _str);
    }

    // no copy constructor
    // no destructor
};

void test();

int main() {
    test();

    return 0;
}

void test() {
    BadImpl a{"Hello"};
    auto b = a;
} // will called dezstructor on both instances -> UB
