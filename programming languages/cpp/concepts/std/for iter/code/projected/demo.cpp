#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people {
        {"Bob", 30},
        {"Alice", 20},
        {"John", 25}
    };

    auto&&proj = &Person::age;
    auto&& result = std::invoke(proj, *people.begin());
    std::cout << result << std::endl;

    std::ranges::sort(
        people,
        std::ranges::less(),
        &Person::age
    );
    for (const auto &p: people) {
        std::cout
            << "{name: '" << p.name
            << "', age: " << p.age << "}" << std::endl;
    }

    return 0;
}
