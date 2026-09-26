#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <format>
#include <thread>
#include <chrono>

#include "framing.hpp"

int main() {
    constexpr auto path{"/tmp/demo_uds.sock"};

    const int fd{socket(AF_UNIX, SOCK_STREAM, 0)};
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    int attempts{0};
    while (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        if (++attempts > 50) {
            perror("connect");
            return 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "[client] connect to server\n" << std::flush;

    for (int i{1}; i <= 3; ++i) {
        if (!send_msg(fd, std::format("hello: #{}", i))) {
            perror("send_msg");
            return 1;
        }

        const auto answer{recv_msg(fd)};
        if (!answer) {
            std::cerr << "[client] server closed connection\n";
            return 1;
        }
        std::cout << std::format("[client] answer from server: {}\n",*answer) << std::flush;
    }

    close(fd);
    std::cout << "[client] done, close connection\n" << std::flush;

    return 0;
}
