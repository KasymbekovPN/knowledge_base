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

int main(int argc, char const *argv[]) {
    try {
        Person tom {"Tom", 1234};
        tom.print();

    } catch(const AgeException& e) {
        std::cerr << e.getMessage() << '\n';
    }
    
    return 0;
}
