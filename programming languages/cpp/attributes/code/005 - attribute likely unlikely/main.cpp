
#include <iostream>
#include <ostream>

namespace {
    int handleError() { return -1; }

    int process(const int x) {
        if (x > 0) [[likely]] {
            return 2 * x;
        } else [[unlikely]] {
            return handleError();
        }
    }

    enum class Status { OK, ERROR };

    int doWork(const Status status) {
        switch (status) {
            case Status::OK: [[likely]]
                return process(42);
            case Status::ERROR: [[unlikely]]
                return handleError();
        }
    }

}

int main() {
    std::cout << doWork(Status::OK) << std::endl;

    return 0;
}
