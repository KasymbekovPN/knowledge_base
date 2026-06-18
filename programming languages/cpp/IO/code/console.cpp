#include <iostream>

int main() {
    int x{};
    std::cin >> x; // ввод с форматированием
    std::cout << "value: " << x << "\n"; // буферизованный вывод
    std::cerr << "Error!\n"; // небуферизованный (unitbuf)
    std::clog << "Log message\n"; // буферизованный вывод ошибок

    return 0;
}
