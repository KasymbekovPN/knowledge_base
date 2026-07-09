#include <atomic>
#include <chrono>
#include <iostream>
#include <format>
#include <mutex>
#include <thread>
#include <vector>

std::mutex log_mutex;
std::atomic<int> shared_counter{0};

void worker(const int id, const int iterations) {
    for (int i{}; i < iterations; ++i) {
        int local_value{id * 100 + i}; // <-- breakpoint здесь, разный для каждого потока
        shared_counter.fetch_add(1);

        {
            std::lock_guard<std::mutex> lock(log_mutex);
            std::cout << std::format(
                "worker {}: iteration= {}, local_value = {}",
                id,
                i,
                local_value);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    const int NUM_WORKERS{4};
    std::vector<std::thread> threads;

    for (int i{}; i < NUM_WORKERS; ++i) {
        threads.emplace_back(worker, i, 10);
    }

    for (auto& t : threads) {
        t.join();
    }

    std::cout << std::format("total countre: {}", shared_counter.load());
    return 0;
}


/*

###
lldb .\build\debug\app.exe
breakpoint set --file main.cpp --line 14

### Scheduler-locking
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
break main.cpp:14
run
set scheduler-locking on
next
next
info threads

### Non-stop mode
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe
set non-stop on
set target-async on
break main.cpp:14
run
info threads
thread 2
continue

*/
