#include <iostream>
#include <mutex>

std::mutex mtx;
int shared_data;

void safe_function();

int main() {
    safe_function();

    return 0;
}

void safe_function() {
    // capture the mutex
    std::lock_guard<std::mutex> lock(mtx);

    // critical section
    shared_data++;
    std::cout << "shared_data => " << shared_data << std::endl;
} // destructor calling -> mtx.unlock()
