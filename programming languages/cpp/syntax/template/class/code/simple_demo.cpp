#include <iostream>

template <class T>
class Person {

private:
    T id;
    std::string name;

public:
    explicit Person(T id, std::string name) noexcept: id{id}, name{name} {}
    void print() const noexcept {
        std::cout
            << "{id: " << id
            << ", name: " << name
            << "}" << std::endl;
    }
};

int main(int argc, char const *argv[]) {
    Person<int> tom {12345, "Tom"};
    tom.print();

    Person<std::string> bob {"qwery", "Bob"};
    bob.print();

    return 0;
}
