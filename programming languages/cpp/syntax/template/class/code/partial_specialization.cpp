#include <iostream>

template<typename T, typename P>
class Person {

private:
    T id;
    std::string name;
    P phone;

public:
    explicit Person(std::string name, P phone) noexcept:
        name{name},
        phone{phone} {}

    void setId(T id) const noexcept {
        this->id = id;
    }

    void print() const noexcept {
        std::cout
            << "{id: '" << id << "'"
            << ", name: " << name
            << ", phone: " << phone
            << "}" << std::endl;
    }
};

template<typename P>
class Person<unsigned, P>{

private:
    static inline unsigned count {};
    unsigned id;
    std::string name;
    P phone;

public:
    explicit Person(std::string name, P phone) noexcept:
        name{name},
        phone{phone} {

        id = ++count;
    }

    void print() const noexcept {
        std::cout
            << "{id: " << id
            << ", name: " << name
            << ", phone: " << phone
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person<std::string, std::string> tom {"Tom", "12345"};
    tom.print();

    Person<unsigned, std::string> bob {"Bob", "56789"};
    bob.print();

    return 0;
}
