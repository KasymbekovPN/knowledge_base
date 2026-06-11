#include <iostream>
#include <string>
#include <string_view>

int main() {
    const std::string_view a {"a"};
    const std::string_view b {"b"};
    const std::string_view c {"c"};

    std::cout << std::boolalpha;
    std::cout
        << a << " < "  << b << " => "
        << (a < b) << std::endl;
    std::cout
        << a << " > "  << b << " => "
        << (a > b) << std::endl;
    std::cout
        << a << " == " << b << " => "
        << (a == b) << std::endl;
    std::cout << std::noboolalpha;

    std::cout
        << b << ".compare(" << a << ")" 
        << " => " << b.compare(a)
        << std::endl;
    std::cout
        << b << ".compare(" << b << ")" 
        << " => " << b.compare(b)
        << std::endl;
    std::cout
        << b << ".compare(" << c << ")" 
        << " => " << b.compare(c)
        << std::endl;

    return 0;
}
