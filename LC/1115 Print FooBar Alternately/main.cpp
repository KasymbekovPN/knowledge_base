#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <semaphore>

namespace {
    class FooBarBlocked {
        static constexpr int MIN_N{3};

        std::mutex mtx;
        std::condition_variable cv;
        int n;
        bool foo_turn;

        public:
            explicit FooBarBlocked(const int n):
                n{n <= 0 ? MIN_N : n},
                foo_turn{true} {}

        void foo(const std::function<void()>& print_foo) {
            for (int i{}; i < n; ++i) {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [this] { return foo_turn; });
                print_foo();
                foo_turn = false;
                cv.notify_all();
            }
        }

        void bar(const std::function<void()>& print_bar) {
            for (int i{}; i < n; ++i) {
                std::unique_lock<std::mutex> lk(mtx);
                cv.wait(lk, [this] { return !foo_turn; });
                print_bar();
                foo_turn = true;
                cv.notify_all();
            }
        }
    };

    class FooBarSemaphore {
        static constexpr int MIN_N{3};

        std::binary_semaphore foo_sem{1};
        std::binary_semaphore bar_sem{0};
        int n;

        public:
            explicit FooBarSemaphore(const int n): n{n <= 0 ? MIN_N : n} {}

            void foo(const std::function<void()>& print_foo) {
                for (int i{}; i < n; ++i) {
                    foo_sem.acquire();
                    print_foo();
                    bar_sem.release();
                }
            }

            void bar(const std::function<void()>& print_bar) {
                for (int i{}; i < n; ++i) {
                    bar_sem.acquire();
                    print_bar();
                    foo_sem.release();
                }
            }
    };

    void start_test0() {
        const auto print_foo = [] { std::cout << "foo "; };
        const auto print_bar = [] { std::cout << "bar\n"; };

        FooBarBlocked foo_bar{5};

        std::thread t1{&FooBarBlocked::bar, &foo_bar, print_bar};
        std::thread t2{&FooBarBlocked::foo, &foo_bar, print_foo};

        t1.join();
        t2.join();
    }

    void start_test1() {
        const auto print_foo = [] { std::cout << "foo "; };
        const auto print_bar = [] { std::cout << "bar\n"; };

        FooBarSemaphore foo_bar{5};

        std::thread t1{&FooBarSemaphore::bar, &foo_bar, print_bar};
        std::thread t2{&FooBarSemaphore::foo, &foo_bar, print_foo};

        t1.join();
        t2.join();
    }

}

int main() {
    std::cout << "### start_test0 ###\n";
    start_test0();

    std::cout << "### start_test1 ###\n";
    start_test1();

    return 0;
}
