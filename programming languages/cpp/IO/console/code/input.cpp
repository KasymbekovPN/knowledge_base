#include <iostream>

int main(int argc, char const *argv[]){
    int age;
    int size;
    double weight;
    
    std::cout << "Input age: ";
    std::cin >> age;

    std::cout << "Input size and weight: ";
    std::cin >> size >> weight;

    std::cout << "Age <= " << age << std::endl;
    std::cout << "Size <= " << size << std::endl;
    std::cout << "Weight <= " << weight << std::endl;

    return 0;
}
