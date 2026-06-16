#include <iostream>

class Person0 {

public:
    std::string name;
    unsigned age;

    Person0(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout << "[Person0, both] name: '" << name << "', age: " << age << " - created" << std::endl;
    }

    Person0(std::string p_name) {
        name = p_name;
        age = 42;
        std::cout << "[Person0, name] name: '" << name << "', age: " << age << " - created" << std::endl;
    }

    Person0() {
        name = "NoName";
        age = 42;
        std::cout << "[Person0, none] none: '" << name << "', age: " << age << " - created" << std::endl;
    }

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl;
    }
};

class Person1 {

public:
    std::string name {};
    unsigned age {};

    Person1(std::string p_name, unsigned p_age) {
        name = p_name;
        age = p_age;
        std::cout << "[Person1] name: '" << name << "', age: " << age << " - created" << std::endl;
    }

    Person1(std::string p_name): Person1(p_name, 42) {
        std::cout << "[Person1] Second" << std::endl;
    }

    Person1(): Person1("NoName", 42) {
        std::cout << "[Person1] Third" << std::endl;
    }

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person0 p00{"Tom", 18};
    Person0 p01("Bob");
    Person0 p02;
    Person1 p10{"Tommy", 19};
    Person1 p11("Bobby");
    Person1 p12;

    p00.print();
    p01.print();
    p02.print();
    p10.print();
    p11.print();
    p12.print();

    return 0;
}
