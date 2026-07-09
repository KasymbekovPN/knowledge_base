#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex cout_mutex;
std::atomic<int> shared_counter{0};

void worker(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        int local_value = id * 100 + i;   // <-- breakpoint здесь, разный для каждого потока
        shared_counter.fetch_add(1);

        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "worker " << id << ": iteration " << i
                      << ", local_value=" << local_value << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::vector<std::thread> workers;
    const int num_workers = 4;

    for (int id = 0; id < num_workers; ++id) {
        workers.emplace_back(worker, id, 10);
    }

    for (auto& t : workers) {
        t.join();
    }

    std::cout << "Total counter = " << shared_counter.load() << std::endl;
    return 0;
}

/*

###
lldb .\build\debug\app.exe



###
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe

 */
