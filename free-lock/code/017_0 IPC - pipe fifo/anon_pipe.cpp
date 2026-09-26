#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <iostream>
#include <format>
#include <string>
#include <cstdlib>
#include <thread>
#include <chrono>

namespace {
    template <typename... Args>
    void print_log(const int pid, std::format_string<Args...> fmt, Args&&... args) {
        std::cout << "[" << pid << "] " << std::format(fmt, std::forward<Args>(args)...) << std::flush;
    }
}

int main(int argc, char *argv[]) {
    constexpr int TIMEOUT_S{5};
    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

    int pipe_fd[2];
    if (pipe(pipe_fd) != 0) {
        perror("pipe");
        return 1;
    }

    const pid_t pid{fork()};
    print_log(pid, "PID: {}\n", pid);
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(pipe_fd[0]);
        constexpr auto msg{"hello from child"};
        write(pipe_fd[1], msg, strlen(msg));
        close(pipe_fd[1]);

        std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

        _exit(0);
    }

    close(pipe_fd[1]);
    char buf[256] = {};
    const ssize_t n{read(pipe_fd[0], buf, sizeof(buf) - 1)};
    // std::cout << std::format("[parent] read {} bytes: {}\n", n , buf) << std::flush ;
    print_log(pid, "[parent] read {} bytes: {}\n", n, buf);
    close(pipe_fd[0]);
    waitpid(pid, nullptr, 0);

    std::this_thread::sleep_for(std::chrono::seconds(TIMEOUT_S));

    return 0;
}
