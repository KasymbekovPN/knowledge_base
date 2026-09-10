#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace {

    class FLocked {
        std::mutex mtx;
        std::condition_variable cv;
        int state;

    public:
        explicit FLocked(): state{0} {}

        void first(const std::function<void()>& print_first) {
            std::unique_lock<std::mutex> lk(mtx);
            print_first();
            state = 1;
            cv.notify_all();
        }

        void second(const std::function<void()>& print_second) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return state == 1; });
            print_second();
            state = 2;
            cv.notify_all();
        }

        void third(const std::function<void()>& print_third) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return state == 2; });
            print_third();
        }
    };

    class FLockFree {
        std::atomic<int> state{0};

    public:
        void first(const std::function<void()>& print_first) {
            print_first();
            state.store(1, std::memory_order_release);
        }

        void second(const std::function<void()>& print_second) {
            while (state.load(std::memory_order_acquire) != 1) {}
            print_second();
            state.store(2, std::memory_order_release);
        }

        void third(const std::function<void()>& print_third) const {
            while (state.load(std::memory_order_acquire) != 2) {}
            print_third();
        }
    };

    void start_test30() {
        const auto print_first = [] { std::cout << "F30 first\n"; };
        const auto print_second = [] { std::cout << "F30 second\n"; };
        const auto print_third = [] { std::cout << "F30 third\n"; };

        FLocked f;
        std::thread t1{&FLocked::first, &f, print_first};
        std::thread t2{&FLocked::second, &f, print_second};
        std::thread t3{&FLocked::third, &f, print_third};

        t3.join();
        t1.join();
        t2.join();
    }

    void start_test31() {
        const auto print_first = [] { std::cout << "F31 first\n"; };
        const auto print_second = [] { std::cout << "F31 second\n"; };
        const auto print_third = [] { std::cout << "F31 third\n"; };

        FLockFree f;
        std::thread t1{&FLockFree::first, &f, print_first};
        std::thread t2{&FLockFree::second, &f, print_second};
        std::thread t3{&FLockFree::third, &f, print_third};

        t3.join();
        t1.join();
        t2.join();
    }
}

int main() {
    start_test30();
    start_test31();

    return 0;
}
