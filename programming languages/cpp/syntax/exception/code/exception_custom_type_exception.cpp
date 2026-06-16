#include <iostream>

class person_error: public std::exception {

private:
    std::string message;

public:
    explicit person_error(const std::string&) noexcept;
    const char* what() const noexcept override;
};

person_error::person_error(const std::string& message) noexcept:
    message{message} {};

const char* person_error::what() const noexcept {
    return message.c_str();
}

class Person {

private:
    std::string name;
    unsigned age;

public:
    explicit Person(std::string, unsigned);
    void print() const noexcept;
};

Person::Person(std::string name, unsigned age): 
    name{name},
    age{age} {

    if (!this->age || this->age > 110) {
        throw person_error{"Invalid age"};
    }
}

void Person::print() const noexcept {
    std::cout
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

void test_person(std::string, unsigned);

int main(int argc, char const *argv[]) {
    test_person("Tom", 42);
    test_person("Sam", 1234);

    return 0;
}

void test_person(std::string name, unsigned age) {
    try {
        Person person {name, age};
        person.print();

    } catch(const person_error& e) {
        std::cerr << "person_error <= " << e.what() << std::endl;
    } catch(...) {
        std::cerr << "Unknown exception" << std::endl;
    }    
}
