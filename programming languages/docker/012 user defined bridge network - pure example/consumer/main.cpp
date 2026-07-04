#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <format>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    const int PORT{5000};

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creating socket\n";
        return 1;
    }

    // Разрешаем повторное использование порта сразу после рестарта контейнера
    int OPT{1};
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &OPT, sizeof(OPT));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << std::format("Error binding socket to {}\n", PORT);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << std::format("Error listening for connections\n");
        return 1;
    }

    std::cout << std::format("Listening for connection: {}\n", PORT);

    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);
        if (client_fd < 0) {
            std::cerr << std::format("Error accepting connection\n");
            continue;
        }

        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            std::string received{buffer, static_cast<size_t>(bytes_read)};
            // убираем перевод строки на конце, если он есть
            while (!received.empty() &&
                (received.back() == '\r' || received.back() == '\n')) {
                received.pop_back();
            }
            std::cout << std::format("Received: {}\n", received);

            std::string response = std::format("ECHO: {}\n", received);
            send(client_fd, response.c_str(), response.length(), 0);
        }
        close(client_fd);
    }

    close(server_fd);

    return 0;
}
