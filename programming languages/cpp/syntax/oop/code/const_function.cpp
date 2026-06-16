#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    void print() const;
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    tom.print();

    return 0;
}
