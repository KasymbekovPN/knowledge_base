#include <iostream>
#include <string>

class AgeException {

private:
    std::string message;

public:
    explicit AgeException(std::string) noexcept;
    virtual std::string getMessage() const noexcept;
};

AgeException::AgeException(std::string message) noexcept:
    message{message} {}

std::string AgeException::getMessage() const noexcept {
    return message;
}

class MaxAgeException: public AgeException {

private:
    unsigned maxAge;

public:
    explicit MaxAgeException(std::string, unsigned);
    std::string getMessage() const noexcept override;
};

MaxAgeException::MaxAgeException(std::string message, unsigned maxAge):
    AgeException{message},
    maxAge{maxAge} {}

std::string MaxAgeException::getMessage() const noexcept {
    return AgeException::getMessage() + " -- max age should be " + std::to_string(maxAge);
}

class Person {

private:
    static const unsigned MAX_AGE {110};
    std::string name;
    unsigned age;

public:
    Person(std::string, unsigned);
    void print() const noexcept;
};

Person::Person(std::string name, unsigned age): name{name} {
    if (!age) {
        throw AgeException{"Invalid age"};
    }
    if (age > MAX_AGE) {
        throw MaxAgeException{"Invalid age", MAX_AGE};
    }
    this->age = age;
}

void Person::print() const noexcept {
    std::cout 
        << "{name: " << name
        << ", age: " << age
        << "}" << std::endl;
}

void test(std::string, unsigned);

int main(int argc, char const *argv[]) {
    ::test("Tom", 42);
    ::test("Bob", 0);
    ::test("Paul", 1234);
    
    return 0;
}

void test(std::string name, unsigned age) {
    try {
        Person person {name, age};
        person.print();

    } catch(const MaxAgeException& e) {
        std::cerr << "MaxAgeException <= " << e.getMessage() << std::endl;
    } catch(const AgeException& e) {
        std::cerr << "AgeException <= " << e.getMessage() << std::endl;
    }
}
