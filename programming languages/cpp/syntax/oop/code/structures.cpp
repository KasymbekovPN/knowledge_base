#include <iostream>

struct person {
    std::string name;
    unsigned age;
};

int main(int argc, char const *argv[]) {
    person tom;
    tom.name = "Tom";
    tom.age = 34;

    std::cout
        << "{name: " << tom.name
        << ", age: " << tom.age
        << "}" << std::endl;

    return 0;
}
