#pragma 
#include "net_compat.h"
#include <chrono>
#include <stdexcept>

#ifdef _WIN32
    #include <io.h>
#else
    #include <fcntl.h>
    #include <sys/select.h>
#endif

// Перевод сокета в (не)блокирующий режим — кросс-платформенно.
inline bool set_nonblocking(socket_t _fd, bool _on) {
#ifdef _WIN32
    u_long mode = _on ? 1 : 0;
    return ioctlsocket(_fd, FIONBIO, &mode) == 0;
#else
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags < 0) return false;
    flags = on ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    return ::fcntl(fd, F_SETFL, flags) == 0;
#endif
}

// Установка таймаутов на чтение и запись (в миллисекундах).
inline void set_io_timeout(socket_t _fd, int _ms) {
#ifdef _WIN32
    DWORD tv = static_cast<DWORD>(_ms); // Windows: DWORD миллисекунд
    setsockopt(
        _fd,
        SOL_SOCKET,
        SO_RCVTIMEO,
        reinterpret_cast<const char*>(&tv),
        sizeof(tv)
    );
    setsockopt(
        _fd,
        SOL_SOCKET,
        SO_SNDTIMEO,
        reinterpret_cast<const char*>(&tv),
        sizeof(tv)
    );
#else
    timeval tv;                          // POSIX: struct timeval
    tv.tv_sec  = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    ::setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
}

// connect() с таймаутом. Возвращает подключённый сокет или INVALID_SOCK.
inline socket_t connect_to(
    const char* _host,
    const char* _port,
    int connect_ms = 5000,
    int io_ms = 5000
)
{
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* res = nullptr;
    if (::getaddrinfo(_host, _port, &hints, &res) != 0) return INVALID_SOCK;

    socket_t fd = INVALID_SOCK;

    for (addrinfo* p{res}; p != nullptr; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == INVALID_SOCK) continue;

        // Неблокирующий connect, чтобы ограничить ожидание через select.
        set_nonblocking(fd, true);
        int rc{
            connect(fd, p->ai_addr, static_cast<int>(p->ai_addrlen))
        };

        // мгновенно подключились (редко)
        if (rc == 0) {
            set_nonblocking(fd, false);
            break;
        }

        // Ожидаем готовности записи в течение connect_ms.
        fd_set wset;
        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        FD_SET(fd, &wset);
        timeval tv;
        tv.tv_sec = connect_ms / 1000;
        tv.tv_usec = (connect_ms % 1000) * 1000;

        // На POSIX первый аргумент select — максимальный fd + 1;
        // на Windows он игнорируется, можно передать 0.
        #ifdef _WIN32
            rc = select(0, nullptr, &wset, nullptr, &tv);
        #else
            rc = ::select(fd + 1, nullptr, &wset, nullptr, &tv);
        #endif

        if (rc > 0) {
            // select сказал «готов», но надо проверить, успешен ли connect:
            // читаем отложенную ошибку сокета через SO_ERROR.
            int so_err{};
            #ifdef _WIN32
                int len = sizeof(so_err);
            #else
                socklen_t len = sizeof(so_err);
            #endif
            getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_err), &len);
            if (so_err == 0) {
                set_nonblocking(fd, false);
                break;
            }
        }

        // rc == 0 → таймаут; rc < 0 → ошибка select; so_err != 0 → connect не удался
        close_socket(fd);
        fd = INVALID_SOCK;
    }

    freeaddrinfo(res);

    if (fd != INVALID_SOCK) set_io_timeout(fd, io_ms); // таймауты на recv/send

    return fd;
}
