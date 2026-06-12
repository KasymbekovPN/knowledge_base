#include <iostream>

template<typename T>
struct Printable {
    void print() const {
        std::cout
            << static_cast<const T*>(this)->getData()
            << std::endl;
    }
};

struct Message: Printable<Message> {
    std::string data{"hello !!!"};
    const std::string& getData() const {
        return data;
    }
};

struct Number: Printable<Number> {
    int value{42};
    int getData() const {
        return value;
    }
};

int main() {
    Message m;
    m.print();

    Number n;
    n.print();
    
    return 0;
}
