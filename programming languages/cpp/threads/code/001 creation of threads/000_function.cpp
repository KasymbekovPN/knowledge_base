#include <iostream>
#include <thread>

void do_work() {
    std::cout << "do_work executed." << std::endl;
}

int main() {
    std::thread t(do_work);
    t.join();
   
    return 0;
}
