---
tags:
  - programming-language
  - docker
---
# Безопасность: непривилегированный пользователь и сканирование уязвимостей

## Почему нельзя запускать процессы от root

Если не указать иначе, **все процессы внутри контейнера по умолчанию выполняются от root** (UID 0). Это создаёт конкретный риск: если в приложении найдут уязвимость (переполнение буфера, RCE через зависимость и т.д.) и злоумышленник получит возможность выполнять код внутри контейнера — он получит **root внутри контейнера**. Дальше это может быть использовано для:

- Эскалации через известные уязвимости container runtime (побег из контейнера на хост)
- Модификации файлов, смонтированных как volume, с полными правами
- Более широкого доступа к возможностям ядра (capabilities), которые Docker по умолчанию не отбирает у root-процесса

Root **внутри** контейнера — не то же самое, что root **на хосте**, но это ещё один рубеж защиты, который лучше не отдавать бесплатно.

## Создание непривилегированного пользователя — Alpine

```dockerfile
FROM alpine:3.20

RUN apk add --no-cache libstdc++ boost-system

# Создаём группу и пользователя без домашней директории и без shell-логина
RUN addgroup -S appgroup && adduser -S appuser -G appgroup

WORKDIR /app
COPY --from=builder /src/build/consumer .

# Владелец файлов приложения — наш пользователь, а не root
RUN chown -R appuser:appgroup /app

# Переключаемся на непривилегированного пользователя для всех
# последующих инструкций и для самого запуска контейнера
USER appuser

EXPOSE 5000
ENTRYPOINT ["./consumer"]
```

Флаги `-S` в `addgroup`/`adduser` — создать **системного** пользователя (без пароля, без домашней директории, не для интерактивного логина). Это специфика Alpine/BusyBox (`adduser` там из BusyBox, а не GNU coreutils).

## То же самое на Debian/Ubuntu — синтаксис другой

```dockerfile
RUN groupadd -r appgroup && useradd -r -g appgroup appuser
```

## Важный практический нюанс — порты ниже 1024

Непривилегированный пользователь **не может** забиндиться на порт ниже 1024 (это ограничение самого Linux, а не Docker). Наш `consumer` слушает 5000 — с этим проблем нет. Но если бы приложение слушало, скажем, 80 — от `appuser` это бы упало с `Permission denied`. Решения: слушать порт ≥1024 внутри контейнера и делать проброс `-p 80:8080` наружу (как раз тот случай из прошлого вопроса про несовпадающие номера портов), либо явно дать процессу capability `CAP_NET_BIND_SERVICE` через `docker run --cap-add`.

## Проверка на практике

```powershell
docker build -t consumer:secure .
docker run -d --name consumer --network mynet consumer:secure
docker exec consumer id
```

Ожидаемо:

```
uid=100(appuser) gid=101(appgroup)
```

вместо `uid=0(root)`.

## Сканирование образов на уязвимости

Даже если сам код без багов, **базовый образ и его пакеты** могут содержать известные CVE (например, устаревшая версия `libssl` с известной уязвимостью). Сканеры проверяют состав образа против баз данных известных уязвимостей.

### Docker Scout — встроен в Docker Desktop

```powershell
docker scout cves consumer:secure
```

Покажет список найденных CVE с severity (critical/high/medium/low), затронутым пакетом и рекомендацией (обычно — обновить базовый образ или конкретный пакет).

Быстрая сводка при обычном `docker build` — Docker Desktop сам подсказывает в выводе `What's next`, если видит проблемы (ты уже видел эту строчку в своих логах: `docker ai "help me fix..."` — Scout работает похожим образом, просто для CVE).

### Trivy — популярный open-source сканер

```powershell
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock aquasec/trivy image consumer:secure
```

Trivy часто используют в CI/CD как gate — например, "не пушить образ, если есть уязвимости severity CRITICAL":

```powershell
trivy image --severity CRITICAL --exit-code 1 consumer:secure
```

Возвращает ненулевой exit code, если что-то нашлось — удобно встраивать в pipeline, чтобы автоматически блокировать релиз небезопасного образа.

## Почему `alpine` тут в плюсе

Меньше пакетов внутри → меньше потенциальных CVE по определению. Это ещё один аргумент в пользу alpine из прошлого сравнения (alpine vs slim vs full) — не только про размер, но и про сокращение поверхности атаки за счёт меньшего количества установленного ПО.

## Практическое правило для регулярной проверки

Сканирование — это не разовая проверка, а процесс: новые CVE находят постоянно в уже существующих пакетах. Стандартная практика — пересобирать образы на актуальной версии базового образа регулярно (даже если код приложения не менялся), и гонять сканер как часть CI при каждой сборке.

## Пример

### consumer/main.cpp
```cpp
#include <boost/asio.hpp>  
#include <boost/asio/co_spawn.hpp>  
#include <boost/asio/detached.hpp>  
#include <boost/asio/use_awaitable.hpp>  
#include <iostream>  
#include <format>  
#include <future>  
#include <string>  
  
// Обрабатывает одного клиента как отдельную корутину.  
// Несколько клиентов могут обслуживаться конкурентно на одном потоке io_context.  
boost::asio::awaitable<void> handle_client(boost::asio::ip::tcp::socket socket) {  
    try {  
        for (;;) {  
            char data[1024];  
            std::size_t n{co_await socket.async_read_some(  
                boost::asio::buffer(data),  
                boost::asio::use_awaitable  
            )};  
            std::string received{data, n};  
            while (!received.empty()  
                && (received.back() == '\n' || received.back() == '\r')) {  
                received.pop_back();  
            }            std::cout << std::format("Received: {}\n", received);  
  
            std::string response{std::format("ECHO: {}\n", received)};  
            co_await boost::asio::async_write(  
                socket,  
                boost::asio::buffer(response),  
                boost::asio::use_awaitable  
            );  
        }    } catch (const std::exception& e) {  
        // Обрыв соединения клиентом бросает исключение из async_read_some — это нормальный сценарий  
        std::cout << std::format("Client turned off: {}\n", e.what());  
    }}  
  
boost::asio::awaitable<void> listener() {  
    const int PORT{5000};  
  
    auto executor = co_await boost::asio::this_coro::executor;  
    boost::asio::ip::tcp::acceptor acceptor(  
        executor,  
        boost::asio::ip::tcp::endpoint(  
            boost::asio::ip::tcp::v4(),  
            PORT  
        )  
    );    std::cout << std::format("Consumer listen to port {}\n", PORT);  
    for (;;) {  
        boost::asio::ip::tcp::socket socket = co_await acceptor.async_accept(  
            boost::asio::use_awaitable  
        );  
        // Каждый новый клиент — отдельная независимая корутина  
        boost::asio::co_spawn(  
            executor,  
            handle_client(std::move(socket)),  
            boost::asio::detached  
        );  
    }}  
  
int main() {  
    try {  
        boost::asio::io_context io_context(1);  
        boost::asio::co_spawn(io_context, listener(), boost::asio::detached);  
        io_context.run();  
    } catch (const std::exception& e) {  
        std::cerr << std::format("Exception: {}\n", e.what());  
        return 1;  
    }  
    return 0;  
}
```

### consumer/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(consumer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
find_package(Boost REQUIRED COMPONENTS system)  
find_package(Threads REQUIRED)  
  
add_executable(consumer main.cpp)  
target_link_libraries(consumer PRIVATE Boost::system Threads::Threads)
```

### consumer/Dockfile
```Dockerfile
**# ---- этап сборки ----  
FROM alpine:3.20 AS builder  
  
RUN apk add --no-cache build-base cmake boost-dev  
  
WORKDIR /src  
COPY CMakeLists.txt main.cpp ./  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- финальный этап ----  
FROM alpine:3.20  
  
RUN apk add --no-cache libstdc++ boost-system  
  
# Непривилегированный пользователь  
RUN addgroup -S appgroup && adduser -S appuser -G appgroup  
  
WORKDIR /app  
COPY --from=builder /src/build/consumer .  
  
# Владелец файлов приложения — appuser, не root  
RUN chown -R appuser:appgroup /app  
  
USER appuser  
  
EXPOSE 5000  
ENTRYPOINT ["./consumer"]**
```

### producer/main.cpp
```cpp
#include <boost/asio.hpp>  
#include <boost/asio/co_spawn.hpp>  
#include <boost/asio/detached.hpp>  
#include <boost/asio/use_awaitable.hpp>  
#include <iostream>  
#include <format>  
#include <string>  
  
boost::asio::awaitable<void> send_request(const std::string host, const std::string message) {  
    auto executor{co_await boost::asio::this_coro::executor};  
  
    const std::string PORT{"5000"};  
    boost::asio::ip::tcp::resolver resolver{executor};  
    auto endpoints{co_await resolver.async_resolve(  
        host,  
        PORT,  
        boost::asio::use_awaitable  
    )};  
  
    boost::asio::ip::tcp::socket socket{executor};  
    co_await boost::asio::async_connect(  
        socket,  
        endpoints,  
        boost::asio::use_awaitable  
    );  
    std::cout << std::format("Connected to {}:{}, send: {}\n", host, PORT, message);  
  
    std::string request{message + "\n"};  
    co_await boost::asio::async_write(  
        socket,  
        boost::asio::buffer(request),  
        boost::asio::use_awaitable);  
  
    char data[1024];  
    std::size_t n{co_await socket.async_read_some(  
        boost::asio::buffer(data),  
        boost::asio::use_awaitable)};  
    std::cout << std::format("Answer: {}\n", std::string(data, n));  
}  
  
int main(int argc, char *argv[]) {  
    std::string host = (argc > 1) ? argv[1] : "consumer";  
    std::string message = (argc > 2) ? argv[2] : "Привет от producer!";  
  
    try {  
        boost::asio::io_context io_context(1);  
        boost::asio::co_spawn(io_context, send_request(host, message), boost::asio::detached);  
        io_context.run();  
    } catch (const std::exception& e) {  
        std::cerr << std::format("Error: {}\n", e.what());  
        return 1;  
    }    return 0;  
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(producer LANGUAGES CXX)  
  
set(CMAKE_CXX_STANDARD 20)  
set(CMAKE_CXX_STANDARD_REQUIRED ON)  
  
find_package(Boost REQUIRED COMPONENTS system)  
find_package(Threads REQUIRED)  
  
add_executable(producer main.cpp)  
target_link_libraries(producer PRIVATE Boost::system Threads::Threads)
```

### producer/Dockerfile
```Dockerfile
FROM alpine:3.20 AS builder  
  
RUN apk add --no-cache build-base cmake boost-dev  
  
WORKDIR /src  
COPY CMakeLists.txt main.cpp ./  
  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
FROM alpine:3.20  
  
RUN apk add --no-cache libstdc++ boost-system  
  
RUN addgroup -S appgroup && adduser -S appuser -G appgroup  
  
WORKDIR /app  
COPY --from=builder /src/build/producer .  
RUN chown -R appuser:appgroup /app  
  
USER appuser  
  
ENTRYPOINT ["./producer"]
```

## Пересобираем и проверяем непривилегированного пользователя

```powershell
docker build -t consumer:secure ./consumer
docker build -t producer:secure ./producer

docker network create mynet

docker run -d --name consumer --network mynet consumer:secure
docker exec consumer id
```

Ожидаемо:

```
uid=100(appuser) gid=101(appgroup) groups=101(appgroup)
```

Проверяем, что producer тоже работает от appuser и приложение по-прежнему функционирует:

```powershell
docker run --rm --network mynet producer:secure consumer "Проверка от appuser"
```

Ожидаемо:

```
Подключились к consumer:5000, отправляем: Проверка от appuser
Ответ: ECHO: Проверка от appuser
```

Если это сработало — значит, `bind()` на порт 5000 у `consumer` прошёл успешно от непривилегированного пользователя (порт 5000 ≥ 1024, поэтому проблем с правами не возникло, как я упоминал).

## Сканирование на уязвимости

Docker Scout встроен в Docker Desktop — можно запускать прямо так:

```powershell
docker scout cves consumer:secure
```

Если Scout ещё не активирован в твоём Docker Desktop, он попросит войти в аккаунт Docker Hub при первом запуске — это нормально, сканирование локальное, просто требует авторизации для доступа к базе CVE.

Альтернатива без привязки к Docker Hub аккаунту — Trivy через отдельный контейнер:

```powershell
docker run --rm -v /var/run/docker.sock:/var/run/docker.sock aquasec/trivy image consumer:secure
```

Первый запуск скачает базу данных уязвимостей (может занять минуту), дальше — быстрее.

## Что ожидать в результатах

Для `alpine:3.20` с минимальным набором пакетов (`libstdc++`, `boost-system`) результат обычно будет скромным — возможно несколько LOW/MEDIUM CVE в самой базе alpine, редко что-то CRITICAL, если версия alpine актуальная. Если увидишь HIGH/CRITICAL — обрати внимание, к какому пакету это относится: если это `boost-system` или `libstdc++` — стоит проверить, есть ли более новая версия `alpine:3.20.x` с патчем, или переключиться на более свежий тег (`alpine:3.21` и т.д.).
