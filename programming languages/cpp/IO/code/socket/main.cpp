#include "socketbuf.h"
#include "connect.h"

#include <iostream>
#include <format>
#include <string>

int main() {
    try {
        NetInit net;

        const char* host{"example.com"};
        socket_t fd{connect_to(host, "80")};
        if (fd == INVALID_SOCK) {
            std::cerr << std::format("Failed to connect to {}\n", host);
            return 1;
        }

        socketbuf buf{fd};
        std::iostream sock{&buf};

        sock
            << std::format("GET /get HTTP/1.1\r\nHost: {}\r\nConnection: close\r\n\r\n", host)
            << std::flush;

        std::string line;
        while(std::getline(sock, line)) { 
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            std::cout << line << '\n';
        }

        // Разбираемся, почему цикл закончился.
        switch (buf.status()) {
            case socketbuf::Status::Closed:
                // штатное завершение: сервер закрыл соединение после ответа
                break;
            case socketbuf::Status::Timeout:
                std::cerr << "\n[Reading timeout]\n";
                break;
            case socketbuf::Status::Error:
                std::cerr << std::format("\n[Socket error: {}]\n", last_error());
                break;
            case socketbuf::Status::Ok:
                break;
        }

        close_socket(fd);
    } catch(const std::exception& e) {
        std::cerr << std::format("Exception: {}\n", e.what());
        return 1;
    }

    return 0;
}
