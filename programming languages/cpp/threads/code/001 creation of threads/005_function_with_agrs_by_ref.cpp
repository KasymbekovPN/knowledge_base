#include <iostream>
#include <thread>

void inc(int& _input) {
    _input++;
}

int main() {
    int value{0};
    std::thread t{inc, std::ref(value)};
    t.join();

    std::cout << "value: " << value << std::endl;
    
    return 0;
}
