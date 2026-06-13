#include <iostream>
#include <thread>

void worker() {
    std::cout << "Worker executed" << std::endl;
}

int main() {
    {
        std::thread t{worker};
        t.join();
    }

    {
        std::jthread t{worker};
    }

    return 0;
}
