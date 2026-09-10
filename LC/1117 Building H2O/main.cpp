#include <iostream>
#include <thread>
#include <semaphore>
#include <barrier>
#include <functional>
#include <mutex>

namespace {
    class H20CV {
        static constexpr int O_QUANTITY {1};
        static constexpr int H_QUANTITY {2};

        std::mutex mtx;
        std::condition_variable cv;
        int h_count{0};
        int o_count{0};
        int departed_count{0};

    public:
        void hydrogen(const std::function<void()>& release_hydrogen) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&] { return h_count < H_QUANTITY; });
            h_count++;
            release_hydrogen();
            cv.notify_all();

            wait_for_triple_and_reset(lk);
        }

        void oxygen(const std::function<void()>& release_oxygen) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&] { return o_count < O_QUANTITY; });
            o_count++;
            release_oxygen();
            cv.notify_all();

            wait_for_triple_and_reset(lk);
        }

    private:
        [[nodiscard]] bool is_triple_completed() const {
            return o_count == O_QUANTITY && h_count == H_QUANTITY;
        }

        void wait_for_triple_and_reset(std::unique_lock<std::mutex>& lk) {
            cv.wait(lk, [&] { return is_triple_completed(); });

            departed_count++;
            if (departed_count == H_QUANTITY + O_QUANTITY) {
                h_count = o_count = 0;
                departed_count = 0;
            }
            cv.notify_all();
        }
    };

    class H20SEM {
        std::counting_semaphore<2> hydrogen_sem{2};
        std::binary_semaphore oxygen_sem{1};
        std::barrier<> sync_point{3};

    public:
        void hydrogen(const std::function<void()>& release_hydrogen) {
            hydrogen_sem.acquire();
            release_hydrogen();
            sync_point.arrive_and_wait();
            hydrogen_sem.release();
        }

        void oxygen(const std::function<void()>& release_oxygen) {
            oxygen_sem.acquire();
            release_oxygen();
            sync_point.arrive_and_wait();
            oxygen_sem.release();
        }
    };

    void start_test0() {
        const auto release_hydrogen = [] { std::cout << "H"; };
        const auto release_oxygen = [] { std::cout << "O"; };

        std::vector<std::thread> threads;

        H20CV h2o;
        for (int i{}; i < 3; i++) {
            threads.emplace_back(&H20CV::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20CV::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20CV::oxygen, &h2o, release_oxygen);
        }

        for (auto& t: threads) {
            t.join();
        }

        std::cout << std::endl;
    }

    void start_test1() {
        const auto release_hydrogen = [] { std::cout << "H"; };
        const auto release_oxygen = [] { std::cout << "O"; };

        std::vector<std::thread> threads;

        H20SEM h2o;
        for (int i{}; i < 3; i++) {
            threads.emplace_back(&H20SEM::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20SEM::hydrogen, &h2o, release_hydrogen);
            threads.emplace_back(&H20SEM::oxygen, &h2o, release_oxygen);
        }

        for (auto& t: threads) {
            t.join();
        }

        std::cout << std::endl;
    }

}

int main() {
    start_test0();
    start_test1();

    return 0;
}
