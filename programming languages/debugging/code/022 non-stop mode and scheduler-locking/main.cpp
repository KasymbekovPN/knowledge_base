#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mutex_a;
std::mutex mutex_b;

void task_one() {
    std::cout << "task_one: locking mutex_a\n";
    std::lock_guard<std::mutex> lock_a{mutex_a};

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "task_one: locking mutex_b\n";
    std::lock_guard<std::mutex> lock_b{mutex_b};

    std::cout << "task_one: got both locks\n";

}

void task_two() {
    std::cout << "task_two: locking mutex_b\n";
    std::lock_guard<std::mutex> lock_b{mutex_b};

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "task_two: locking mutex_a\n";
    std::lock_guard<std::mutex> lock_a{mutex_a};

    std::cout << "task_two: got both locks\n";

}

int main() {
    {
        std::jthread t1{task_one};
        std::jthread t2{task_two};
    }

    std::cout << "Unreachable\n";
    return 0;
}

/*

###
Get-Process app | Select-Object Id
lldb -p 24596
thread backtrace all

###
Get-Process app | Select-Object Id
gdb -p 24596
thread apply all bt

*/
