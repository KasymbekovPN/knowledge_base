#include <iostream>
#include <thread>

struct Worker {
    static const size_t SIZE{100000};
    int value{};

    void run() {
        for (size_t i{}; i < SIZE; i++) {
            ++value;
        }
    }
};

int main() {
    Worker w;
    
    std::thread t0 {&Worker::run, std::ref(w)};
    std::thread t1 {&Worker::run, std::ref(w)};

    t0.join();
    t1.join();

    std::cout << "w.value: " << w.value << std::endl;
    
    return 0;
}
