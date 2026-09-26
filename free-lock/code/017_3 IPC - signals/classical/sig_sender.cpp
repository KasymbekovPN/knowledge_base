#include <csignal>
#include <iostream>
#include <format>
#include <fstream>

int main(int argc, char *argv[]) {
    constexpr auto path{"/tmp/sig_receiver.pid"};
    std::ifstream pidfile(path);
    int target_pid;
    pidfile >> target_pid;
    std::cout << std::format("[SENDER] pid= {}, from= {}\n", target_pid, path) << std::flush;

    kill(target_pid, SIGUSR1);

    return 0;
}
