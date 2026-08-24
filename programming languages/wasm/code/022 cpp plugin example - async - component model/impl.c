/*

Реализация example:netcheck/check#tcp-ping для wasm32-wasip2. В
отличие от Варианта 1, здесь ГОСТЬ сам открывает TCP-соединение --
это работает только потому, что для таргета wasm32-wasip2 wasi-libc
реально реализует getaddrinfo/socket/connect поверх компонентного
интерфейса wasi:sockets (проверил вживую: под --target=wasm32-wasip1
эти же вызовы даже не объявлены в заголовках -- "call to undeclared
function"). boost::asio здесь НЕ используется: живая попытка собрать
его под этот таргет упала на нескольких местах, которые Asio
предполагает как данность на POSIX-платформах, а WASI (даже p2) не
предоставляет -- pause() (нет сигналов), errno-код ESHUTDOWN, заголовок
net/if.h. Поэтому здесь -- голые блокирующие POSIX-сокеты, без event
loop'а: настоящий async в госте потребовал бы работать напрямую с
wasi:sockets на уровне pollable-ресурсов компонентной модели, в
обход POSIX-прослойки -- это отдельная, более объёмная задача.

*/

#include "generated/netcheck_plugin.h"

#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1e6;
}

bool exports_example_netcheck_check_tcp_ping(netcheck_plugin_string_t *host,
                                             uint16_t port,
                                             double *ret,
                                             exports_example_netcheck_check_check_error_t *err) {
    // host->ptr не гарантированно NUL-terminated -- копируем.
    char* hostname = (char*)malloc(host->len + 1);
    memcpy(hostname, host->ptr, host->len);
    hostname[host->len] = '\0';

    char port_str[8];
    int n = 0;
    {
        unsigned p = port;
        char tmp[8];
        int i = 0;
        if (p == 0) tmp[i++] = '0';
        while (p > 0) {
            tmp[i++] = (char)('0' + (p % 10));
            p /= 10;
        }
        while (i > 0) port_str[n++] = tmp[--i];
        port_str[n] = '\0';
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res = NULL;
    double t0 = now_ms();
    int gai_err = getaddrinfo(hostname, port_str, &hints, &res);
    free(hostname);
    if (gai_err != 0) {
        err->tag = EXPORTS_EXAMPLE_NETCHECK_CHECK_CHECK_ERROR_RESOLVE_FAILED;
        netcheck_plugin_string_dup(&err->val.resolve_failed, gai_strerror(gai_err));
        return false;
    }

    int fd = -1;
    int connected = 0;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) continue;
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) {
            connected = 1;
            break;
        }
        close(fd);
        fd = -1;
    }

    freeaddrinfo(res);
    if (!connected) {
        err->tag = EXPORTS_EXAMPLE_NETCHECK_CHECK_CHECK_ERROR_CONNECT_FAILED;
        netcheck_plugin_string_dup(&err->val.connect_failed, "could not set TCP-connection");
        return false;
    }

    double t1 = now_ms();
    close(fd);

    *ret = t1 - t0;
    return true;
}
