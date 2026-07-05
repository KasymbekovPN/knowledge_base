---
tags:
  - programming-language
  - docker
---
![[multistage_build_dag.svg|697]]

## Синтаксис стадий

Каждый `FROM` начинает новую стадию. Именование — через `AS`:

```dockerfile
FROM alpine:3.20 AS base
FROM base AS deps
FROM base AS tools
FROM deps AS builder
FROM alpine:3.20 AS final
```

Стадию можно наследовать не только от внешнего образа (`alpine:3.20`), но и **от другой стадии этого же Dockerfile** (`FROM base AS deps`) — тогда она получает всё содержимое родительской стадии на момент её завершения и продолжает с этой точки.

## `COPY --from` — перенос файлов между стадиями

Это ключевой механизм всей техники. Синтаксис:

```dockerfile
COPY --from=<имя_стадии> <путь_в_стадии> <путь_в_текущей_стадии>
```

Пример из нашего `consumer`:

```dockerfile
COPY --from=builder /src/build/consumer .
```

Копирует **только конкретный файл** (готовый бинарник) из стадии `builder` в текущую (`final`) — весь остальной "мусор" builder-стадии (компилятор, заголовки, промежуточные объектники) просто не попадает в итоговый образ, потому что `COPY --from` копирует только явно указанное.

**`--from` может ссылаться не только на стадию Dockerfile, но и на внешний образ:**

```dockerfile
COPY --from=nginx:1.25 /etc/nginx/nginx.conf /app/reference-nginx.conf
```

Docker скачает `nginx:1.25` только ради того, чтобы взять один файл — сама стадия при этом не выполняется, просто используется как источник файлов.

## Собираем конкретную стадию — флаг `--target`

Не обязательно всегда собирать Dockerfile целиком до последней стадии:

```powershell
docker build --target builder -t consumer:debug .
```

Это соберёт всё **до** стадии `builder` включительно и остановится там — удобно для отладки: получаешь образ с компилятором и всеми инструментами внутри, чтобы зайти и вручную проверить, что пошло не так на этапе сборки.

## Параллельная сборка независимых стадий

Это то, что видно на графе: `deps` и `tools` обе стартуют от `base`, но **не зависят друг от друга**. BuildKit (движок по умолчанию с 2023+, тот самый, что показывает `[+] Building` с параллельными шагами в твоих логах) автоматически анализирует граф зависимостей стадий и **собирает независимые ветки конкурентно**, а не строго последовательно сверху вниз.

```dockerfile
FROM alpine:3.20 AS base
RUN apk add --no-cache curl

FROM base AS deps
RUN apk add --no-cache boost-dev

FROM base AS tools
RUN apk add --no-cache protobuf-dev

FROM base AS builder
COPY --from=deps /usr/include /usr/include
COPY --from=tools /usr/include /usr/include
COPY . .
RUN cmake -B build -S . && cmake --build build
```

В логе сборки это будет видно как одновременный прогресс `[2/6] deps` и `[3/6] tools` — реальный выигрыш по времени, если у тебя многоядерный CPU и стадии достаточно тяжёлые (например, компиляция разных наборов зависимостей).

**Важный нюанс:** параллелизм автоматический, тебе не нужно ничего специально настраивать — просто структурируй Dockerfile так, чтобы независимые части действительно были независимыми стадиями, а не последовательными `RUN` внутри одной.

## Продвинутая техника — `COPY --from` конкретных файлов, не всей стадии

Не обязательно копировать директориями — можно точечно:

```dockerfile
COPY --from=builder /src/build/consumer /app/consumer
COPY --from=builder /src/config.json /app/config.json
```

Это даёт полный контроль над тем, что попадает в финальный слой — ничего лишнего не просочится случайно.

## Пропуск неиспользуемых стадий

Если стадия объявлена, но на неё никто не ссылается через `--from` и она не является целевой (`--target`) — BuildKit **вообще её не выполнит**, даже если она есть в файле. Полезно для "стадий-заготовок", которые используются не всегда (например, стадия с тестами, которую гоняешь только в CI через `--target test`, но которая не участвует в обычной сборке образа).

## Применительно к нашему producer/consumer

Наш текущий Dockerfile для `consumer` — простой двухстадийный (`builder` → финал). Более продвинутая версия с параллелизмом могла бы выглядеть так:

```dockerfile
FROM alpine:3.20 AS base
RUN apk add --no-cache boost-dev

FROM base AS builder
WORKDIR /src
COPY CMakeLists.txt main.cpp .
RUN apk add --no-cache build-base cmake && \
    cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

FROM alpine:3.20 AS final
RUN apk add --no-cache libstdc++ boost-system
WORKDIR /app
COPY --from=builder /src/build/consumer .
ENTRYPOINT ["./consumer"]
```

Тут реального выигрыша от параллелизма нет (всего одна "тяжёлая" ветка), но если бы у нас были, скажем, отдельно `consumer` и `producer` в одном общем Dockerfile с общей `base` — вот тогда параллельная сборка независимых бинарников дала бы реальный прирост скорости.


## Пример

```
.../
├── Dockerfile
├── consumer/
│   ├── CMakeLists.txt
│   └── main.cpp
└── producer/
    ├── CMakeLists.txt
    └── main.cpp
```

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

### producer/CMakeLists.txt
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

### Dockerfile
```Dockerfile
# ---- общая база: тяжёлые build-инструменты ставим один раз ----  
FROM alpine:3.20 AS base  
RUN apk add --no-cache build-base cmake boost-dev  
  
# ---- параллельная стадия: сборка consumer ----  
FROM base AS builder-consumer  
WORKDIR /src  
COPY consumer/CMakeLists.txt .  
COPY consumer/main.cpp .  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- параллельная стадия: сборка producer ----  
FROM base AS builder-producer  
WORKDIR /src  
COPY producer/CMakeLists.txt .  
COPY producer/main.cpp .  
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && \  
    cmake --build build  
  
# ---- финальный образ: только consumer ----  
FROM alpine:3.20 AS consumer  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-consumer /src/build/consumer .  
EXPOSE 5000  
ENTRYPOINT ["./consumer"]  
  
# ---- финальный образ: только producer ----  
FROM alpine:3.20 AS producer  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-producer /src/build/producer .  
ENTRYPOINT ["./producer"]  
  
# ---- демонстрационная стадия: оба бинарника сразу ----  
# Это ПОСЛЕДНЯЯ стадия в файле — именно её Docker соберёт по умолчанию,  
# без явного --target. Раз она зависит и от builder-consumer, и от  
# builder-producer — обе эти стадии будут собираться в рамках ОДНОГО  
# вызова docker build, и вот тут параллелизм BuildKit станет заметен.  
FROM alpine:3.20 AS both  
RUN apk add --no-cache libstdc++ boost-system  
WORKDIR /app  
COPY --from=builder-consumer /src/build/consumer .  
COPY --from=builder-producer /src/build/producer .
```
## Проверяем параллелизм на практике

```powershell
docker build -t combined:1.0 --progress=plain --no-cache .
```

`--no-cache` тут специально — чтобы гарантированно увидеть полную сборку обеих веток, а не переиспользование кэша. В выводе с `--progress=plain` обрати внимание на **временные метки** у шагов `builder-consumer` и `builder-producer` — они должны идти внахлёст, а не строго последовательно одна за другой, если у тебя больше одного CPU-ядра доступно Docker Desktop.

## Собираем отдельные production-образы через `--target`

```powershell
docker build --target consumer -t consumer:3.0 .
docker build --target producer -t producer:3.0 .
```

Обрати внимание: при повторном вызове второй командой стадия `base` (и, если ничего не менялось, `builder-producer`) уже возьмётся из кэша, собранного первым вызовом — общая `base`-стадия переиспользуется между разными целевыми образами.

## Проверка, что оба бинарника реально работают

```powershell
docker network create mynet

# используем "both" образ, но запускаем в нём только consumer
docker run -d --name consumer --network mynet combined:1.0 ./consumer

docker logs consumer
# Consumer слушает порт 5000...

# из того же образа запускаем producer
docker run --rm --network mynet combined:1.0 ./producer consumer "Привет из объединённого образа!"
```

Ожидаемо:

```
Подключились к consumer:5000, отправляем: Привет из объединённого образа!
Ответ: ECHO: Привет из объединённого образа!
```

## Сравни размеры

```powershell
docker images
```

Интересно сопоставить: `combined:1.0` (содержит оба бинарника) должен быть лишь немного больше, чем `consumer:3.0` или `producer:3.0` по отдельности — потому что base-слои Alpine и рантайм-библиотек (`libstdc++`, `boost-system`) общие и физически не дублируются на диске (это та же экономия за счёт переиспользования слоёв, которую мы разбирали в самом начале).
