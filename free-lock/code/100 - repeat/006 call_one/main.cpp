#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

namespace {
    class Logger {
        static inline std::once_flag flag_{};
        static inline Logger* inst_{nullptr};

    public:
        static Logger* get_instance() {
            std::call_once(flag_, [] { inst_ = new Logger(); });
            return inst_;
        }

        void println(const std::string& line) const {
            std::cout << std::format("{}\n", line) << std::flush;
        }
    };

    void worker(const int value) {
        const auto* logger = Logger::get_instance();
        logger->println(std::to_string(value));
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i{}; i < 10; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t: threads) t.join();

    return 0;
}
