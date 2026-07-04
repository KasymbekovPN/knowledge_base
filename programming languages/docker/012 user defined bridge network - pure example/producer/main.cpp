#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <format>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Хост по умолчанию — "consumer", это ИМЯ контейнера в user-defined bridge сети,
    // резолвится через встроенный DNS Docker'а
    const std::string HOST{argc > 1 ? argv[1] : "consumer"};
    const std::string MESSAGE{argc > 2 ? argv[2] : "Hello from producer!"};
    const int PORT{5000};

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result{nullptr};
    int status{getaddrinfo(
            HOST.c_str(),
            std::to_string(PORT).c_str(),
            &hints,
            &result
        )
    };
    if (status != 0) {
        std::cerr << std::format("Can not resolve host: {}::{}\n", HOST, gai_strerror(status));
        return 1;
    }

    int sock_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock_fd < 0) {
        std::cerr << "Can not create socket\n";
        freeaddrinfo(result);
        return 1;
    }

    if (connect(sock_fd, result->ai_addr, result->ai_addrlen) < 0) {
        std::cerr << std::format("Can not connect to socket {}:{}\n", HOST, PORT);
        freeaddrinfo(result);
        close(sock_fd);
        return 1;
    }
    freeaddrinfo(result);

    std::cout << std::format("Connected to {}:{}, send: {}\n", HOST, PORT, MESSAGE);

    std::string request{MESSAGE + '\n'};
    send(sock_fd, request.c_str(), request.size(), 0);

    char buffer[1024] = {0};
    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        std::cout << std::format("Answer: {}\n", std::string(buffer, static_cast<size_t>(bytes_read)));
    }

    close(sock_fd);
    return 0;
}
