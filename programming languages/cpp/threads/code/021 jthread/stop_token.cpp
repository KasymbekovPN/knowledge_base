#include <iostream>
#include <thread>

void worker(std::stop_token _stoken)  {
    while (!_stoken.stop_requested()) { 
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "stopped\n";
}

int main() {
    std::jthread t{worker};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
