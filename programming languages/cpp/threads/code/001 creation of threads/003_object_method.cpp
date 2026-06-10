#include <iostream>
#include <thread>

struct Worker {
    int data{};

    void run() {
        data++;
    }
};

int main() {
    Worker w;
    std::thread t{&Worker::run, &w};
    t.join();
    std::cout << "w.data " << w.data << std::endl;

    std::thread t1{&Worker::run, std::ref(w)};
    t1.join();

    std::cout << "w.data " << w.data << std::endl;   
    
    return 0;
}
