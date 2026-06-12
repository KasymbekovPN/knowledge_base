#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers1 = {1, 2, 3};
    std::vector<int> numbers2 = {1, 2, 3};
    std::vector<int> numbers3 = {1, 2, 4};
    std::vector<int> numbers4 = {1, 2};
    std::vector<int> numbers5 = {1, 2, 3, 4};

    std::cout
        << "numbers1 == numbers2 -> "
        << std::boolalpha
        << (numbers1 == numbers2)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers1 != numbers3 -> "
        << std::boolalpha
        << (numbers1 != numbers3)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers1 < numbers3 -> "
        << std::boolalpha
        << (numbers1 < numbers3)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers4 <= numbers1 -> "
        << std::boolalpha
        << (numbers4 <= numbers1)
        << std::noboolalpha
        << std::endl;

    std::cout
        << "numbers5 > numbers1 -> "
        << std::boolalpha
        << (numbers5 > numbers1)
        << std::noboolalpha
        << std::endl;

    return 0;
}
