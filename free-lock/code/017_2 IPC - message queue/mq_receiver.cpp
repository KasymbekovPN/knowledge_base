#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <vector>
#include <chrono>
#include <thread>

int main() {
    constexpr auto name{"/demo_mq"};

    mqd_t mq;
    int attempts{};
    while ((mq = mq_open(name, O_RDONLY)) == static_cast<mqd_t>(-1)) {
        if (++attempts > 50) {
            perror("mq_open");
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    // struct mq_attr {};
    mq_attr attr{};
    mq_getattr(mq, &attr);

    std::vector<char> buf(attr.mq_msgsize);
    for (int i{}; i < 3; ++i) {
        unsigned priority{};
        const ssize_t n{mq_receive(mq, buf.data(), buf.size(), &priority)};
        if (n < 0) {
            perror("mq_receive");
            break;
        }
        std::cout << std::format("[receiver] took (priority={}) {}\n",
            priority,
            std::string(buf.data(), n)
        ) << std::flush;
    }

    mq_close(mq);
    mq_unlink(name);

    return 0;
}
