#pragma once

// Framing с префиксом длины: [uint32 длина в network byte order][payload].
// SOCK_STREAM не хранит границы сообщений, поэтому их задаём сами.

#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

inline constexpr std::uint32_t max_msg_size{64 * 1024};

// write может записать меньше, чем просили, дописываем в цикле.
inline bool write_all(const int fd, const void* data, std::size_t size) {
    auto p{static_cast<const char*>(data)};
    while (size > 0) {
        const ssize_t n{write(fd, p, size)};
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        p += n;
        size -= static_cast<std::size_t>(n);
    }
    return true;
}

// read может вернуть часть данных, дочитываем ровно size байт.
// false: EOF или ошибка.
inline bool read_all(const int fd, void* data, std::size_t size) {
    auto p{static_cast<char*>(data)};
    while (size > 0) {
        const ssize_t n{read(fd, p, size)};
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (n == 0) return false;
        p += n;
        size -= static_cast<std::size_t>(n);
    }
    return true;
}

inline bool send_msg(const int fd, const std::string_view msg) {
    if (msg.size() > max_msg_size) return false;
    const std::uint32_t len{htonl(static_cast<std::uint32_t>(msg.size()))};
    return write_all(fd, &len, sizeof(len)) && write_all(fd, msg.data(), msg.size());
}

// nullopt: соединение закрыто, ошибка или слишком длинное сообщение.
inline std::optional<std::string> recv_msg(const int fd) {
    std::uint32_t len{};
    if (!read_all(fd, &len, sizeof(len))) return std::nullopt;
    len = ntohl(len);
    if (len > max_msg_size) return std::nullopt;

    std::string msg(len, '\0');
    if (!read_all(fd, msg.data(), len)) return std::nullopt;
    return msg;
}
