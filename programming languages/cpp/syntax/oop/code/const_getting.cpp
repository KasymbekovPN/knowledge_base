#include <iostream>

class Person {

private:
    std::string name;
    unsigned age;

public:
    Person(std::string _name, unsigned _age);
    const std::string& getName() const;
    const unsigned* getAge() const;
    void print() const;
};

Person::Person(std::string _name, unsigned _age): name(_name), age(_age) {}

const std::string& Person::getName() const {
    return name;
}

const unsigned* Person::getAge() const {
    return &age;
}

void Person::print() const{
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    const Person tom {"Tome", 42};
    std::cout
        << "{name: " << tom.getName()
        << ", age: " << *tom.getAge()
        << "}" << std::endl;

    return 0;
}
