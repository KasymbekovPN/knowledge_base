#include <iostream>
#include <thread>

struct Worker {
    void run() const {
        std::cout << "Worker::run" << std::endl;
    }
};

int main() {
    Worker w;
    std::thread t{&Worker::run, &w};
    t.join();
    
    return 0;
}
