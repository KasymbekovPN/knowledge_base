#include <iostream>
#include <vector>

int main(int argc, char const *argv[]) {
    std::vector<int> numbers {0, 1, 2, 3, 4};
    std::vector<int>::iterator it0 = numbers.begin();
    std::vector<int>::iterator it1 = it0;

    std::cout << "#####" << std::endl;
    std::cout << "it0 <= " << *it0 << std::endl;
    std::cout << "it1 <= " << *it1 << std::endl;

    it0++;
    std::cout << "#####" << std::endl;
    std::cout << "it0 <= " << *it0 << std::endl;
    std::cout << "it1 <= " << *it1 << std::endl;
    std::cout
        << "it0 != it1 <= " << std::boolalpha
        << (it0 != it1) << std::noboolalpha << std::endl;

    it0--;
    std::cout << "#####" << std::endl;
    std::cout << "it0 <= " << *it0 << std::endl;
    std::cout << "it1 <= " << *it1 << std::endl;
    std::cout
        << "it0 != it1 <= " << std::boolalpha
        << (it0 != it1) << std::noboolalpha << std::endl;

    return 0;
}
