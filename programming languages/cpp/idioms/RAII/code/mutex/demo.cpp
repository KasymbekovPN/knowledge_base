#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;
int counter {0};

void safe_inc();

int main(int argc, char const *argv[]) {
    safe_inc();

    return 0;
}

void safe_inc() {
    std::lock_guard<std::mutex> lock(mtx); // capture the mutex

    ++counter;
    std::cout << "counter => " << counter << std::endl;
} // unlocking in destructor, in case of exception too
