#include <iostream>
#include <concepts>

struct Animal {
    virtual ~Animal() = default;
};

struct Dog: Animal {};

struct Cat: Animal {};

struct Bulldog: Dog {};

struct Human {};

template<std::derived_from<Animal> T>
void make_sound(const T&);

template<typename T>
requires std::derived_from<T, Animal>
void process(T&);

int main() {
    Animal animal;
    Cat cat;
    Dog dog;
    Bulldog bulldog;
    Human human;

    make_sound(animal);
    make_sound(cat);
    make_sound(dog);
    make_sound(bulldog);
    // make_sound(human); // Error

    process(animal);
    process(cat);
    process(dog);
    process(bulldog);
    // process(human); // Error

    return 0;
}

template<std::derived_from<Animal> T>
void make_sound(const T& _input) {
    std::cout << "Some animal sound" << std::endl;
}

template<typename T>
requires std::derived_from<T, Animal>
void process(T& _input) {
    std::cout << "Processing an aninal" << std::endl;
}
