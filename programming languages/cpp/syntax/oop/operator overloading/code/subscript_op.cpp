#include <iostream>
#include <string>

class Person {

private:
    std::string name;
    std::string company;
    unsigned age;

public:
    Person(std::string, unsigned, std::string);
    void print() const;
    std::string operator[](unsigned) const;
};

Person::Person(std::string name, unsigned age, std::string company):
    name{name},
    age{age},
    company{company} {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << ", company: " << company
        << "}" << std::endl;
}

std::string Person::operator[](unsigned index) const {
    switch (index) {
        case 0: return name;
        case 1: return std::to_string(age);
        case 2: return company;
        default: return "Bad index";
    }
}

int main(int argc, char const *argv[]) {
    Person p {"Tom", 42, "Company"};
    p.print();

    const unsigned INDEXES[] {0, 1, 2, 3};
    for (auto idx : INDEXES) {
        std::cout << "idx: " << idx << " <=> " << p[idx] << std::endl;
    }
    
    return 0;
}
