#include <iostream>
#include <thread>

void start_worker() {
    std::cout << "Worker executed" << std::endl;
}

int main() {
    std::thread t{start_worker};
    t.join();

    return 0;
}
