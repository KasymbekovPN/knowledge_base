#include <iostream>
#include <thread>

class Logger {

public:
    Logger() {
        std::cout
            << "[" << std::this_thread::get_id() << "] Logger created\n";
    }

    ~Logger() {
        std::cout
            << "[" << std::this_thread::get_id() << "] Logger destroyed\n";
    }
};

thread_local size_t counter{};
thread_local Logger logger;

void worker () {
    ++counter;
    ++counter;
    std::cout
        << "[" << std::this_thread::get_id()
        << "] counter: " << counter << "\n";
}

int main() {
    std::thread t0{worker};
    std::thread t1{worker};

    t0.join();
    t1.join();

    return 0;
}
