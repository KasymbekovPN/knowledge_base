#include <iostream>

class Person {

public:
    const std::string name;
    unsigned age;

    Person(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
    }
};

int main(int argc, char const *argv[]) {
    return 0;
}
