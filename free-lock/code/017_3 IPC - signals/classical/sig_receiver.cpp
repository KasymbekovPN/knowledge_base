#include <csignal>
#include <iostream>
#include <format>
#include <fstream>

namespace {
    volatile sig_atomic_t got_usr1{0};
    volatile sig_atomic_t got_term{0};

    void handler(const int sig) {
        if (sig == SIGUSR1) got_usr1 = 1;
        if (sig == SIGTERM) got_term = 1;
    }
}

int main(int argc, char *argv[]) {
    const auto pid{getpid()};
    std::cout << std::format("[RECEIVER] PID: {}\n", pid) << std::flush;

    constexpr auto path{"/tmp/sig_receiver.pid"};
    std::ofstream pidfile{path};
    pidfile << pid;

    struct sigaction sa{};
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (!got_term) {
        if (got_usr1) {
            got_usr1 = 0;
            /* processing */
        }
        usleep(10000);
    }

    return 0;
}
