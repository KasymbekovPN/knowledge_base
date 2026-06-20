#pragma once

#ifdef _WIN32
    #include <stdexcept>
    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // MSVC: автолинковка; для MinGW — флаг -lws2_32

    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCK = INVALID_SOCKET;

    inline int close_socket(socket_t _socket) { return :: closesocket(_socket); }
    inline int last_error() { return ::WSAGetLastError(); }

    // RAII для WSAStartup/WSACleanup — создать один объект в main().
    struct NetInit {
        NetInit() {
            WSADATA d;
            if (::WSAStartup(MAKEWORD(2, 2), &d) == 0) return;
            throw std::runtime_error{"WSAStartup failed"};
        }
        ~NetInit() { ::WSACleanup(); }
    };

    // Коды ошибок «повторите вызов» / «таймаут».
    inline bool err_would_block(int _error) { return _error == WSAEWOULDBLOCK; }
    inline bool err_interrupted(int _error) { return _error == WSAEINTR; }
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <cerrno>

    using socket_t = int;
    constexpr socket_t INVALID_SOCK = -1;

    inline int  close_socket(socket_t s) { return ::close(s); }
    inline int  last_error()             { return errno; }

    struct NetInit { NetInit() {} ~NetInit() {} };  // на POSIX ничего не нужно

    inline bool err_would_block(int e) { return e == EWOULDBLOCK || e == EAGAIN; }
    inline bool err_interrupted(int e) { return e == EINTR; }
#endif
