#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <format>
#include <cstring>

#include "framing.hpp"

int main() {
    constexpr auto path{"/tmp/demo_uds.sock"};
    unlink(path);

    const int listen_fd{socket(AF_UNIX, SOCK_STREAM, 0)};
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        perror("bind");
        return 1;
    }
    if (listen(listen_fd, 5) != 0) {
        perror("listen");
        return 1;
    }
    std::cout << std::format("[server] listen to '{}'\n", path) << std::flush;

    const int client_fd{accept(listen_fd, nullptr, nullptr)};
    if (client_fd < 0 ) {
        perror("accept");
        return 1;
    }
    std::cout << "[server] client has been connected\n" << std::flush;

    while (const auto msg{recv_msg(client_fd)}) {
        std::cout << std::format("[server] took: {}\n",*msg) << std::flush;
        if (!send_msg(client_fd, std::format("echo: {}", *msg))) {
            perror("send_msg");
            break;
        }
    }

    std::cout << "[server] client disconnected\n" << std::flush;
    close(client_fd);
    close(listen_fd);
    unlink(path);

    return 0;
}
