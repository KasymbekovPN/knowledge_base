#include <iostream>
#include <thread>

void worker(std::stop_token _stoken, int _id) {
    while (!_stoken.stop_requested()) {
        std::cout << "worker " << _id << " working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "worker " << _id << " stopped\n";
}

int main() {
    std::stop_source source;

    std::jthread t0{worker, source.get_token(), 0};
    std::jthread t1{worker, source.get_token(), 1};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    source.request_stop();

    return 0;
}
