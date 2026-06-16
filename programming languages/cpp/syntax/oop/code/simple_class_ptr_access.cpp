#include <iostream>

class Person {
public:
    std::string name {"NoName"};
    unsigned age {42};

    void print() {
        std::cout << "{name: " << name << ", age: " << age << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person person;

    Person* ptr {&person};
    ptr->print();
    
    ptr->name = "Tom";
    ptr->age++;
    ptr->print();

    return 0;
}
