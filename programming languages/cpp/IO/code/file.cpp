#include <iostream>
#include <format>
#include <fstream>

int main() {
    const std::string PATH{"data.txt"};
    std::ofstream out{PATH};
    if (out) {
        out << 42 << ' ' << 3.14 << '\n';
        out.close();

        int n;
        double d;
        std::ifstream in(PATH);
        in >> n >> d;
        std::cout << std::format("n = {}, d = {}", n, d);
    } else {
        std::cout << "Failure to attempt of file creation\n";
    };

    std::ofstream bin("data.bin", std::ios::binary);
    int value = 0xDEADBEEF;
    bin.write(reinterpret_cast<const char*>(&value), sizeof(value));

    return 0;
}
