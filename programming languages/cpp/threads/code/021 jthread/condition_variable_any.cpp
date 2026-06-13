#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable_any cv;
std::queue<int> queue;

void worker(std::stop_token _stoken) {
    std::cout << "Start\n";
    std::unique_lock lock{mtx};
    cv.wait(lock, _stoken, [](){ return !queue.empty(); });
    std::cout << "Done\n";
}

int main() {
    std::jthread t{worker};
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
