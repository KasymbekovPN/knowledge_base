#include <iostream>

class Person {

private:
    class Account {

    public:
        std::string email;
        Account(std::string email = ""): email(email) {}
    };

    std::string name;
    Account account {};
    
public:
    Person(std::string, std::string);
    void print() const;
};

Person::Person(std::string name, std::string email):
    name{name},
    account{Account{email}} {}

void Person::print() const {
    std::cout
        << "{name: " << name
        << "email: " << account.email
        << "}" << std::endl;
}

int main(int argc, char const *argv[]) {
    Person tom {"Tom", "t@localhost.su"};
    tom.print();

    return 0;
}
