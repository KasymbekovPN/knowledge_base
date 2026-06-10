#include <iostream>
#include <thread>

struct Worker {
    const std::string id;
    int value{};
    int result{};

    Worker(const int _value, const std::string _id):
        value{_value},
        id{_id} {}

    void operator()() {
        result = value * value;
        std::cout << "[" << id << "] inner result: " << result << std::endl;
    }

    int getResult() const {
        return result;
    }
};

struct MWorker {
    const std::string id;
    int value{};
    int result{};

    MWorker(const int _value, const std::string _id):
        value{_value},
        id{_id} {}
    MWorker(const MWorker&) = delete;
    MWorker(MWorker&&) = default;

    void operator()() {
        result = value * value;
        std::cout << "[" << id << "] inner result: " << result << std::endl;
    }

    int getResult() const {
        return result;
    }
};

int main() {
    Worker w00(42, "w00");
    std::thread t00 {w00};
    t00.join();
    std::cout << "w00.result " << w00.result << std::endl;

    std::thread t01{Worker{43, "w01"}};
    t01.join();

    std::thread t02{MWorker{44, "w02"}};
    t02.join();

    MWorker w03{45, "w03"};
    // std::thread t03 {w03}; // Error
    std::thread t03 {std::move(w03)};
    t03.join();
    std::cout << "w03.result " << w03.result << std::endl;
    
    return 0;
}
