#include <iostream>
#include <thread>

void worker(std::stop_token _stoken) {
    std::stop_callback scallback{
        _stoken,
        []() { std::cout << "stop required!\n"; }
    };

    while (!_stoken.stop_requested()) {
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::jthread t{worker};
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
