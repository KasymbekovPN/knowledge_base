#include <iostream>
#include <string>

int main() {
    const std::string a {"a"};
    const std::string b {"b"};
    const std::string c {"c"};

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
