---
tags:
  - wasm
---
[[programming languages/wasm/_|<=]]

Во-первых, настоящий ICMP `ping` в WASI не существует ни в одном из вариантов — ни в preview1, ни в компонентной модели (`wasi:sockets`) нет raw-сокетов вообще, это осознанное ограничение песочницы (raw-сокеты требуют привилегий и потенциально позволяют творить что угодно с сетью). Поэтому в обоих примерах — TCP-коннект к `example.com:80` с замером времени, а не ICMP echo. Это самая близкая по духу и реально осуществимая замена.

Во-вторых, `boost::asio` в чистом виде **не собирается под wasi-sdk вообще** — ни под `wasm32-wasip1`, ни под `wasm32-wasip2`. Проверил вживую, не на словах:

```
error: no member named 'pause' in the global namespace           (нет сигналов в WASI)
error: use of undeclared identifier 'ESHUTDOWN'                  (нет такого errno)
fatal error: 'net/if.h' file not found                            (нет ioctl-интерфейсов)
```

Asio исторически рассчитан на полноценный POSIX (epoll/kqueue/select, сигналы, `net/if.h`), а WASI даже в компонентной модели предоставляет принципиально другую, более узкую асинхронную модель (`pollable`-ресурсы), а не файловые дескрипторы с `poll()`. Патченых форков под WASI я не нашёл. Поэтому «асинхронный C++ плагин с boost::asio» в буквальном смысле — только на **хосте**; внутри WASM-гостя async — либо через host-import (вариант 1), либо через прямую работу с `wasi:sockets` без Asio (вариант 2).

**Вариант 1 — чистый wasm, сеть на хосте.** Гость (`plugin_ping_import.c`, обычный `wasm32-wasip1` reactor по контракту `plugin_abi.h`) сам сети не касается — я проверил, что `socket()`/`connect()` для этого таргета даже не объявлены в заголовках wasi-libc («call to undeclared function»). Вместо этого гость зовёт импортированную `host_tcp_ping(ptr,len,port)`. Хост (`host_ping_import.cc`) реализует её через настоящий `boost::asio`: `async_resolve` → `async_connect`, с `steady_timer` в роли асинхронного таймаута — гонка двух async-операций на одном `io_context`, ровно то, ради чего Asio и берут. Прогнал вживую:

```
[хост] boost::asio резолвит и коннектится к example.com:80 ...
Результат plugin_process(): OK: TCP-коннект к example.com:80 занял 19.39 мс
```

Это и архитектурно правильнее: сетевой доступ — capability, которую хост выдаёт явно через импорт, а не то, что гость берёт сам.

**Вариант 2 — компонентная модель, сеть у гостя.** Написал `netcheck.wit` с `tcp-ping: func(host: string, port: u16) -> result<f64, check-error>`, сгенерировал биндинги, реализовал `impl.c` на голых POSIX-сокетах (`getaddrinfo`/`socket`/`connect`) — под `wasm32-wasip2` эти функции реально объявлены и работают поверх `wasi:sockets`. Собралось сразу в компонент (для `wasm32-wasip2` `clang` сам эмитит компонентные импорты, отдельный `wasm-tools component new` не нужен), импорты — `wasi:sockets/tcp-create-socket`, `ip-name-lookup` и т.д., как и ожидалось.

Через `wasmtime run` гостевой код работает полностью:

```
$ wasmtime run -S inherit-network=y -S allow-ip-name-lookup=y --invoke 'tcp-ping("example.com", 80)' netcheck_component.wasm
ok(19.158379)
```

А вот собственный C++-хост (`host_netcheck_component.cc`, тот же паттерн, что в Дне 9) на этот же компонент падает:

```
TRAP: unknown import: `wasi:clocks/monotonic-clock@0.2.6#now` has not been defined
```

Проверил: во всём C API (`wasi.h`, `component/linker.h`, `component/linker.hh`) нет ни единого упоминания `clock` — то есть `add_wasip2()` в этой версии Wasmtime просто не подключает `wasi:clocks`, и никакого способа это сконфигурировать в C API сейчас нет (в отличие от `wasi:cli/environment` из предыдущего сообщения — там мне не хватало `store.context().set_wasi(...)`, и это моя ошибка, а не гэп; я специально это перепроверил и написал выше). А CLI-флаг `-S cli=y` (default) у Rust-биндингов сам бандлит «filesystems, sockets, clocks, and random» одним пакетом — то есть в Rust это либо есть целиком, либо не собирается вовсе, а в C API кто-то забыл прокинуть один конкретный кусок. Чистый, воспроизводимый, второй по счёту живой пример именно того разрыва C API vs Rust API, о котором ты говорил раньше — и на этот раз без всякой двусмысленности с моей стороны.

Sources:

- [Boost.Asio - 1.84.0](https://www.boost.org/doc/libs/1_84_0/doc/html/boost_asio.html)
- [GitHub - WebAssembly/wasi-sockets: WASI API proposal for managing sockets](https://github.com/WebAssembly/wasi-sockets)
- [WebAssembly/wasi-libc](https://github.com/WebAssembly/wasi-libc)

## Пример, pure wasm

### vcpkg.json
```json
{
    "name": "wit-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": ["boost-asio"]
}

```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_ping_import CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system)

include(FetchContent)

set(WASMTIME_VERSION "v47.0.3")
set(WASMTIME_ARCHIVE "wasmtime-${WASMTIME_VERSION}-x86_64-windows-c-api.zip")

FetchContent_Declare(
        wasmtime_c_api
        URL "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/${WASMTIME_ARCHIVE}"
)
FetchContent_MakeAvailable(wasmtime_c_api)

# В архиве нет своего CMakeLists.txt -- это просто заголовки + готовая
# библиотека, поэтому объявляем IMPORTED-таргет вручную. Дальше он
# используется в проекте как обычная зависимость через target_link_libraries.
# Windows-архив содержит только shared-вариант (wasmtime.dll + .dll.lib),
# в отличие от Linux/macOS, где есть статическая libwasmtime.a.
if(WIN32)
    add_library(wasmtime SHARED IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            IMPORTED_IMPLIB "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
else()
    add_library(wasmtime STATIC IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/libwasmtime.a"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
endif()

add_executable(host_ping_import host_ping_import.cpp)
if(WIN32)
    target_link_libraries(host_ping_import PRIVATE wasmtime Boost::system Boost::boost)
    add_custom_command(TARGET host_ping_import POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_ping_import>"
    )
else()
    target_link_libraries(host_ping_import PRIVATE wasmtime pthread dl m Boost::system Boost::boost)
endif()


#add_library(ide OBJECT upper.cpp)

```


### plugin_ping_import.c
```c
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

```

### host_ping_import.cpp
```cpp
// Хост для Варианта 1 -- гость (plugin_ping_import.wasm) сам сети не
// касается, только зовёт импортированную host_tcp_ping. Реальная сеть
// -- здесь, на хосте, через настоящий boost::asio (это обычный нативный
// C++, boost::asio компилируется тут без всяких оговорок -- проблема
// была именно с ЕГО КОМПИЛЯЦИЕЙ ПОД WASI ГОСТЯ, не с хостом).

#include <boost/asio.hpp>

#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <format>
#include <string>
#include <vector>

#include <wasmtime.hh>

namespace {

    const std::string TARGET{"example.com"};

    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }

    // Настоящий асинхронный TCP-коннект через boost::asio: резолвинг и
    // коннект гоняются как async-операции на одном io_context, а
    // steady_timer используется как асинхронный таймаут -- гонка двух
    // async-операций на одном event loop'е, ровно то, ради чего вообще
    // берут asio, а не blocking connect().
    double asio_tcp_ping(const std::string& host, int port) {
        boost::asio::io_context io;
        boost::asio::ip::tcp::resolver resolver{io};
        boost::asio::ip::tcp::socket socket{io};
        boost::asio::steady_timer timer{io};

        bool done{false};
        bool ok{false};
        double elapsed_ms{-1.0};
        auto t0{std::chrono::steady_clock::now()};

        timer.expires_after(std::chrono::seconds(5));
        timer.async_wait([&](const boost::system::error_code& ec) {
            if (ec || done) return;
            done = true;
            boost::system::error_code ignored;
            socket.close(ignored);
        });

        resolver.async_resolve(
            host,
            std::to_string(port),
            [&](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type results) {
                if (ec || done) {
                    done = true;
                    timer.cancel();
                    return;
                }

                boost::asio::async_connect(
                    socket,
                    results,
                    [&](const boost::system::error_code& ec2, const boost::asio::ip::tcp::endpoint&) {
                        if (done) return;
                        done = true;
                        timer.cancel();
                        if (ec2) return;

                        ok = true;
                        elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
                    });

            });

        io.run();
        return ok ? elapsed_ms : -1.0;
    }

}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    auto bytes{read_file("plugin_ping_import.wasm")};

    auto module_result{wasmtime::Module::compile(engine, bytes)};
    if (!module_result) {
        std::cerr << std::format("Module::compile error: {}\n", module_result.err().message());
        return 1;
    }
    wasmtime::Module module{module_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::WasiConfig wasi_config;
    wasi_config.inherit_stdout();
    wasi_config.inherit_stderr();
    auto wasi_result{store.context().set_wasi(std::move(wasi_config))};
    if (!wasi_result) {
        std::cerr << std::format("set_wasi error: {}\n", wasi_result.err().message());
        return 1;
    }

    wasmtime::Linker linker{engine};
    auto define_wasi_result{linker.define_wasi()};
    if (!define_wasi_result) {
        std::cerr << std::format("define_wasi error: {}\n", define_wasi_result.err().message());
        return 1;
    }

    // Регистрируем host_tcp_ping как обычную импортированную функцию --
    // тот же паттерн, что host_log в Дне 5, просто внутри лямбды теперь
    // настоящий сетевой ввод-вывод вместо std::cout.
    wasmtime::Func host_tcp_ping{wasmtime::Func::wrap(
        store,
        [](wasmtime::Caller caller, int32_t ptr, int32_t len, int32_t port) -> double {
            const auto memory{std::get<wasmtime::Memory>(*caller.get_export("memory"))};
            auto data{memory.data(caller.context())};
            std::string host{reinterpret_cast<char*>(data.data() + ptr), static_cast<std::string::size_type>(len)};
            std::cout << std::format("  [host] boost::asio resolve and connect to {}:{} ...\n",
                host, port);
            return asio_tcp_ping(host, port);
        })};
    auto define_result{linker.define(store, "env", "host_tcp_ping", host_tcp_ping)};
    if (!define_result) {
        std::cerr << std::format("linker.define error: {}\n", define_result.err().message());
        return 1;
    }

    auto instance_result{linker.instantiate(store, module)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n", instance_result.err().message());
        return 1;
    }
    wasmtime::Instance instance{instance_result.unwrap()};

    const auto memory{std::get<wasmtime::Memory>(*instance.get(store, "memory"))};
    const auto init_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_init"))};
    const auto alloc_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_alloc"))};
    const auto free_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_free"))};
    const auto progress_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_process"))};
    const auto shutdown_fn{std::get<wasmtime::Func>(*instance.get(store, "plugin_shutdown"))};

    (void)init_fn.call(store, {});

    const int32_t in_ptr{alloc_fn.call(store, {(int32_t)TARGET.size()}).unwrap()[0].i32()};
    std::memcpy(memory.data(store).data() + in_ptr, TARGET.data(), TARGET.size());

    auto res{progress_fn.call(store, {in_ptr, (int32_t)TARGET.size()})};
    if (!res) {
        std::cerr << std::format("plugin_process() -- TRAP: {}\n", res.err().message());
        return 1;
    }

    uint64_t packed{static_cast<uint64_t>(res.unwrap()[0].i64())};
    int32_t out_ptr{static_cast<int32_t>(packed >> 32)};
    int32_t out_len{static_cast<int32_t>(packed & 0xFFFFFFFFu)};
    std::string result{
        reinterpret_cast<char*>(memory.data(store).data() + out_ptr),
        static_cast<std::string::size_type>(out_len)};
    std::cout << std::format("Result of plugin_process(): {}\n", result);

    (void)free_fn.call(store, {out_ptr});
    (void)shutdown_fn.call(store, {});

    return 0;
}

```

## Пример, component model

### vcpkg.json
```json
{
    "name": "wit-demo",
    "version": "1.0.0",
    "builtin-baseline": "a7eda31dc16994fcaa8587982eb833a8695f1b6f",
    "dependencies": []
}

```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(host_net_check_component CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

set(WASMTIME_VERSION "v47.0.3")
set(WASMTIME_ARCHIVE "wasmtime-${WASMTIME_VERSION}-x86_64-windows-c-api.zip")

FetchContent_Declare(
        wasmtime_c_api
        URL "https://github.com/bytecodealliance/wasmtime/releases/download/${WASMTIME_VERSION}/${WASMTIME_ARCHIVE}"
)
FetchContent_MakeAvailable(wasmtime_c_api)

# В архиве нет своего CMakeLists.txt -- это просто заголовки + готовая
# библиотека, поэтому объявляем IMPORTED-таргет вручную. Дальше он
# используется в проекте как обычная зависимость через target_link_libraries.
# Windows-архив содержит только shared-вариант (wasmtime.dll + .dll.lib),
# в отличие от Linux/macOS, где есть статическая libwasmtime.a.
if(WIN32)
    add_library(wasmtime SHARED IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            IMPORTED_IMPLIB "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
else()
    add_library(wasmtime STATIC IMPORTED)
    set_target_properties(wasmtime PROPERTIES
            IMPORTED_LOCATION "${wasmtime_c_api_SOURCE_DIR}/lib/libwasmtime.a"
            INTERFACE_INCLUDE_DIRECTORIES "${wasmtime_c_api_SOURCE_DIR}/include"
    )
endif()

add_executable(host_net_check_component host_net_check_component.cpp)
if(WIN32)
    target_link_libraries(host_net_check_component PRIVATE wasmtime)
    add_custom_command(TARGET host_net_check_component POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${wasmtime_c_api_SOURCE_DIR}/lib/wasmtime.dll"
            "$<TARGET_FILE_DIR:host_net_check_component>"
    )
else()
    target_link_libraries(host_net_check_component PRIVATE wasmtime pthread dl m)
endif()


#add_library(ide OBJECT impl.c)

```

### netcheck.wit
```
// Вариант 2: тот же "ping" (на самом деле TCP-коннект -- см. пояснение
// в финальном ответе про то, почему настоящий ICMP ping недоступен ни
// в одном из вариантов WASI), но теперь гость САМ делает сеть, а не
// зовёт хост -- потому что для world'ов, нацеленных на wasm32-wasip2,
// wasi-libc реально объявляет и реализует socket()/connect() поверх
// компонентного интерфейса wasi:sockets. boost::asio при этом всё
// равно не используется -- он не собирается под этот таргет вообще
// (см. объяснение ниже), поэтому реализация -- на голых POSIX-сокетах.

// wit-bindgen c ./netcheck.wit --out-dir generated

package example:netcheck@0.1.0;

interface check {
  variant check-error {
    resolve-failed(string),
    connect-failed(string),
  }

  // Пытается установить TCP-соединение с host:port и сразу его
  // закрыть. При успехе возвращает время в миллисекундах.
  tcp-ping: func(host: string, port: u16) -> result<f64, check-error>;
}

world netcheck-plugin {
  export check;
}

```

### impl.c
```c
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

```

### host_net_check_component.cpp
```cpp

/*

Хост для Варианта 2 -- Component Model, гость сам делает сеть через
wasi:sockets. Единственное отличие от host_component.cc из Дня 9 --
WasiConfig теперь явно разрешает сеть (inherit_network +
allow_ip_name_lookup), без этого guest получил бы permission-denied
уже на этапе резолвинга адреса.

& "C:\projects\wasi-sdk\wasi-sdk-33.0-x86_64-windows\bin\clang.exe" --% --target=wasm32-wasip2 -mexec-model=reactor -O2 -nostartfiles -Wl,--no-entry -I. -o plugin_core.wasm generated/netcheck_plugin.c impl.c generated/netcheck_plugin_component_type.o

под wasip2 clang уже сразу выдаёт готовый компонент
cp plugin_core.wasm plugin_component.wasm

*/

#include <iostream>
#include <format>
#include <fstream>
#include <vector>

#include <wasmtime/component.hh>
#include <wasmtime/engine.hh>
#include <wasmtime/store.hh>
#include <wasmtime/wasi.hh>

namespace {
    std::vector<uint8_t> read_file(const char* name) {
        std::ifstream file{name, std::ios::binary};
        return std::vector<uint8_t>{
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()};
    }
}

int main(int argc, char *argv[]) {
    wasmtime::Engine engine;
    const char* PATH{argc > 1 ? argv[1] : "plugin_component.wasm"};
    auto bytes{read_file(PATH)};

    auto component_result{wasmtime::component::Component::compile(engine, bytes)};
    if (!component_result) {
        std::cerr << std::format("Compilation error: {}\n", component_result.err().message());
        return 1;
    }
    wasmtime::component::Component component{component_result.unwrap()};

    wasmtime::Store store{engine};
    wasmtime::WasiConfig wasi;
    wasi.inherit_stdout();
    wasi.inherit_stderr();
    wasi.inherit_env();
    // Без этих двух строк -- инстанцирование пройдёт (импорты
    // wasi:sockets всё равно резолвятся), но реальный tcp-ping вернёт
    // permission-denied: сеть в WASI -- capability, которую хост обязан
    // выдать явно, а не то, что гость получает по умолчанию.
    wasi.inherit_network();
    wasi.allow_ip_name_lookup(true);
    auto ctx{store.context()};
    ctx.set_wasi(std::move(wasi)).unwrap();

    wasmtime::component::Linker linker{engine};
    (void)linker.add_wasip2();
    (void)linker.define_unknown_imports_as_traps(component);

    auto instance_result{linker.instantiate(ctx, component)};
    if (!instance_result) {
        std::cerr << std::format("Instantiation error: {}\n", instance_result.err().message());
        return 1;
    }
    wasmtime::component::Instance instance{instance_result.unwrap()};

    auto check_idx{instance.get_export_index(
        ctx,
        nullptr,
        "example:netcheck/check@0.1.0")};

    if (!check_idx) {
        std::cerr << "Interface 'example:netcheck/check@0.1.0' not found";
        return 1;
    }

    auto tcp_ping_idx{instance.get_export_index(
        ctx,
        &*check_idx,
        "tcp-ping")};
    if (!tcp_ping_idx) {
        std::cerr << "function tcp-ping not found";
        return 1;
    }
    wasmtime::component::Func tcp_ping_fn{*instance.get_func(ctx, *tcp_ping_idx)};

    std::string host{"example.com"};
    uint16_t port{80};
    std::vector<wasmtime::component::Val> args{wasmtime::component::Val::string(host), wasmtime::component::Val{port}};
    std::vector<wasmtime::component::Val> results{wasmtime::component::Val{false}};

    if (auto res{tcp_ping_fn.call(ctx, args, results)}; !res) {
        std::cerr << std::format("TRAP: {}\n", res.err().message());
        return 1;
    }

    if (const wasmtime::component::WitResult& wr{results[0].get_result()}; wr.is_ok()) {
        std::cout << std::format("OK: TCP-connection to {}:{} took {} ms\n",
            host,
            port,
            wr.payload()->get_f64());
    } else {
        const wasmtime::component::Variant& errVariant{wr.payload()->get_variant()};
        const auto value{errVariant.value()};
        std::cout << std::format("FAIL ('{}'): ", errVariant.discriminant());
        if (value && value->is_string()) {
            std::cout << value->get_string();
        }
        std::cout << "\n";
    }

    return 0;
}

```
