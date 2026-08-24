// Вариант 1: "чистый" wasm, БЕЗ Component Model, по контракту plugin_abi.h.
// Гость сам сетевым доступом НЕ обладает -- у wasm32-wasip1 (обычный
// "чистый" core-модуль) сокетов нет вообще (socket()/connect() даже не
// объявлены в wasi-libc для этого таргета -- проверил вживую: попытка
// собрать что-то с #include <sys/socket.h> под --target=wasm32-wasip1
// падает с "call to undeclared function 'socket'"). Поэтому гость
// просто зовёт ИМПОРТИРУЕМУЮ хостовую функцию host_tcp_ping, которая
// реализована на хосте через настоящий boost::asio. Это не костыль, а
// правильная архитектура песочницы: сетевой доступ -- капабилити,
// которое явно выдаёт хост через импорт, а не то, что гость берёт сам.

// & "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip1 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -Wl,--export=plugin_abi_version,--export=plugin_init,--export=plugin_alloc,--export=plugin_free,--export=plugin_process,--export=plugin_shutdown -o plugin_ping_import.wasm plugin_ping_import.c

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PLUGIN_ABI_VERSION 1

// Импортируемая хостовая функция: host_tcp_ping(ptr, len, port) -> f64.
// Возвращает время TCP-коннекта в миллисекундах при успехе, отрицательное
// число при неудаче (резолвинг/коннект не прошли). Реализация -- на
// хосте, через boost::asio::async_connect + deadline-таймер.
__attribute__((import_module("env"), import_name("host_tcp_ping")))
extern double host_tcp_ping(const char* host_ptr, int32_t host_len, int32_t port);

static int g_initialized = 0;

__attribute__((export_name("plugin_abi_version")))
int32_t plugin_abi_version(void) { return PLUGIN_ABI_VERSION; }

__attribute__((export_name("plugin_init")))
int32_t plugin_init(void) {
    g_initialized = 1;
    return 0;
}

__attribute__((export_name("plugin_alloc")))
void* plugin_alloc(int32_t size) { return malloc((size_t)size); }

__attribute__((export_name("plugin_free")))
void plugin_free(void* ptr) { free(ptr); }

// // Вход -- имя хоста (например "example.com"), порт всегда 80.
// // Выход -- человекочитаемая строка с результатом.
__attribute__((export_name("plugin_process")))
uint64_t plugin_process(const char* in_ptr, int32_t in_len) {
    if (!g_initialized) return 0;

    double ms = host_tcp_ping(in_ptr, in_len, 80);

    char buf[160];
    int n;
    if (ms >= 0) {
        n = snprintf(buf, sizeof(buf), "OK: TCP-connection to %.*s:80 took %.2f ms", in_len, in_ptr, ms);
    } else {
        n = snprintf(buf, sizeof(buf), "FAIL: could not connect to %.*s:80", in_len, in_ptr);
    }

    if (n < 0) return 0;
    if ((size_t)n > sizeof(buf)) n = (int)sizeof(buf);

    char* out = (char*)plugin_alloc(n);
    if (out == NULL) return 0;
    memcpy(out, buf, (size_t)n);

    return ((uint64_t)(uint32_t)(uintptr_t)out << 32) | (uint32_t)n;
}

__attribute__((export_name("plugin_shutdown")))
void plugin_shutdown(void) { g_initialized = 0; }
