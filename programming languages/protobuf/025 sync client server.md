---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]




---

**`grpc::Server` + `ServerBuilder` — поднятие сервера**

```cpp
OrderServiceImpl service;
ServerBuilder builder;
builder.AddListeningPort(address, grpc::InsecureServerCredentials());
builder.RegisterService(&service);
std::unique_ptr<Server> server = builder.BuildAndStart();
server->Wait();  // блокирует текущий поток до вызова server->Shutdown()
```

`ServerBuilder` — паттерн builder: сначала настраиваешь (порт, credentials, зарегистрированные сервисы, опционально — thread pool, лимиты сообщений), потом `BuildAndStart()` создаёт готовый `grpc::Server` и сразу начинает слушать порт. `Wait()` — синхронный вызов, блокирует поток (поэтому в демо сервер запущен в отдельном `std::thread` — иначе `main()` никогда бы не дошёл до кода клиента). В выводе видно: `[сервер] слушает на 127.0.0.1:50061` — сервер реально забиндил порт и принял соединение от клиента в этом же процессе.

`InsecureServerCredentials()` — без TLS, только для локальной разработки/учебных целей; в проде — `SslServerCredentials()` с сертификатами.

**`Stub` — клиентская сторона**

```cpp
auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
std::unique_ptr<OrderService::Stub> stub = OrderService::NewStub(channel);
```

`Channel` — соединение с сервером (может обслуживать много RPC-вызовов параллельно, переиспользуется). `Stub` — сгенерированный клиентский класс с методом на каждый `rpc` из `.proto` (`stub->GetOrder(...)`, `stub->WatchOrderStatus(...)` и т.д.) — вызов выглядит как обычная функция, а внутри превращается в сетевой запрос.

**`ClientContext` — метаданные и управление одним вызовом**

```cpp
ClientContext context;
Status status = stub->GetOrder(&context, req, &resp);
```

Новый `ClientContext` создаётся **на каждый отдельный RPC-вызов** (не переиспользуется между вызовами — в демо видно 4 отдельных `ClientContext` для 4 разных вызовов). Через него настраиваются deadline (`context.set_deadline(...)`), метаданные запроса (auth-токены, трейсинг), отмена вызова (`context.TryCancel()`). Возвращаемый `Status` — результат вызова: `status.ok()` (в выводе везде `ok=1`), либо код ошибки + сообщение при неуспехе.

**Как выглядят синхронные вызовы по типам (из реально выполнившегося кода)**

Unary — просто вызов с указателем на ответ:

```cpp
Status status = stub->GetOrder(&context, req, &resp);
```

Server streaming — `Stub` возвращает `ClientReader`, читаем в цикле:

```cpp
auto reader = stub->WatchOrderStatus(&context, req);
while (reader->Read(&resp)) { ... }
Status status = reader->Finish();  // Finish() ОБЯЗАТЕЛЕН — иначе utечка/зависание
```

Вывод показывает все 4 статуса (`PENDING`→`DELIVERED`), пришедшие по одному сетевому соединению.

Client streaming — `Stub` возвращает `ClientWriter`, пишем, потом явно завершаем поток:

```cpp
auto writer = stub->BatchUpdateOrder(&context, &ack);
writer->Write(u);       // можно вызывать много раз
writer->WritesDone();   // сигнал серверу "я закончил слать"
Status status = writer->Finish();
```

Сервер в выводе получил оба обновления (`#1`, `#2`) только после того, как весь клиентский поток был вычитан.

Bidi streaming — `ClientReaderWriter`, пишем и читаем в любом порядке (в демо — синхронный ping-pong: пишем, сразу читаем ответ на то же сообщение):

```cpp
auto stream = stub->SyncOrders(&context);
stream->Write(u);
stream->Read(&resp);
stream->WritesDone();
Status status = stream->Finish();
```

**Практическое правило**

Синхронный API прост в написании (линейный код, никаких callback'ов), но каждый вызов блокирует поток на время сетевого round-trip — для высоконагруженного сервера с тысячами одновременных соединений это означает поток на каждый активный запрос. Именно поэтому в проде часто переходят на асинхронный API (`CompletionQueue`) — тема следующего дня плана.

Файл: `demo_grpc_sync.cc`.День 6 практически закрыт (осталась только формальная практика — реализовать unary RPC, что мы уже фактически сделали). Дальше — день 8: streaming подробнее и обработка ошибок/deadlines, либо день 9 (interceptors, TLS, reflection). Что дальше?

--- 


## День 1: Основы protobuf и синтаксис .proto

- [x] Установка: `protobuf-compiler` (protoc) и `libprotobuf-dev` через пакетный менеджер, либо сборка из исходников / vcpkg / Conan. (2026.08.06)
- [x] Синтаксис proto3: `message`, скалярные типы (int32, int64, string, bytes, bool, float/double), номера полей и их роль в wire-формате. (2026.08.06)
- [x] `repeated`, `optional`, `enum`, вложенные сообщения, `oneof`, `map<K,V>`. (2026.08.07)
- [x] Импорты между .proto файлами, `package`, опции `option cc_enable_arenas`, `option optimize_for`. (2026.08.07)
- [x] Практика: описать 3-4 связанных сообщения (например, `User`, `Address`, `Order`) с разными типами полей. (2026.08.07)

## День 2: Генерация C++ кода и API сообщений

- [x] Команда `protoc --cpp_out=. file.proto`, разбор сгенерированных `.pb.h` / `.pb.cc`. (2026.08.07)
- [x] Сгенерированный класс: геттеры/сеттеры, `set_`, `mutable_`, `add_` (для repeated), `has_` (для optional/oneof). (2026.08.07)
- [x] Сериализация: `SerializeToString`, `ParseFromString`, `SerializeToOstream`, `SerializeToArray`. (2026.08.07)
- [x] Отладка: `DebugString()`, `Utf8DebugString()`. (2026.08.07)
- [x] Практика: собрать простую CMake-программу, которая создаёт сообщение, сериализует в файл и читает обратно. (2026.08.07)

## День 3: Wire-формат и эволюция схемы

- [x] Как устроен бинарный формат: tag-length-value, varint-кодирование, zigzag для signed-типов. (2026.08.07)
- [x] Совместимость: почему нельзя переиспользовать номера полей, как безопасно добавлять/удалять/переименовывать поля. (2026.08.07)
- [x] `reserved` для полей и номеров, работа с неизвестными полями (unknown fields). (2026.08.07)
- [x] Разница proto2 vs proto3 vs Protobuf Editions (2023+): `optional` в proto3, дефолтные значения, presence-семантика. (2026.08.07)

## День 4: Производительность и память в C++

- [x] Arena allocation: `google::protobuf::Arena`, зачем нужен, как ускоряет аллокации для больших графов сообщений. (2026.08.08)
- [x] Move-семантика в сгенерированном коде, `Swap()`, избегание лишних копий. (2026.08.08)
- [x] Reflection API (`google::protobuf::Message::GetReflection()`) — для generic-кода, работающего с произвольными типами сообщений. (2026.08.08)
- [x] `Any`, `Timestamp`, `Duration`, `Struct`, `Empty` из `google/protobuf/*.proto` (well-known types). (2026.08.08)

## День 5: Интеграция с CMake / сборочной системой

- [x] `find_package(Protobuf REQUIRED)`, `protobuf_generate_cpp()`. (2026.08.09)
- [x] Альтернатива: `FetchContent`/vcpkg для protobuf как зависимости. (2026.08.09)
- [x] Организация .proto файлов в отдельной директории, генерация в build-директорию, инкрементальная пересборка. (2026.08.09)

## День 6-7: gRPC — основы

- [x] Установка `grpc` и `grpc_cpp_plugin`. (2026.08.09)
- [x] Синтаксис сервисов в .proto: `service`, `rpc`, четыре типа вызовов (unary, server streaming, client streaming, bidi streaming). (2026.08.09)
- [x] Генерация: `--grpc_out` и `--plugin=protoc-gen-grpc-cpp`. (2026.08.09)
- [ ] Синхронный сервер/клиент на C++: `grpc::Server`, `ServerBuilder`, `ClientContext`, `Stub`.
- [ ] Практика: реализовать unary RPC (например, `GetUser(UserRequest) -> UserResponse`), поднять сервер и клиент локально.

## День 8: gRPC — streaming и асинхронность

- [ ] Server streaming и client streaming на практике (например, стрим логов).
- [ ] Bidi streaming.
- [ ] Асинхронный API (`CompletionQueue`) — на уровне понимания, без глубокого погружения.
- [ ] Обработка ошибок: `grpc::Status`, коды ошибок, deadlines/timeouts, retry-политики.
- [ ] Практика: добавить server-streaming метод в сервис дня 6-7.

## День 9: Продвинутые темы

- [ ] Interceptors в gRPC (аутентификация, логирование).
- [ ] TLS/mTLS для защищённых соединений.
- [ ] Reflection service и `grpcurl` для отладки без клиента.
- [ ] Версионирование API и backward-compatibility в реальных сервисах.
- [ ] Практика: подключить `grpcurl` к своему серверу, включить reflection.

## День 10: Итоговый проект

- [ ]  Собрать небольшой сервис целиком: .proto-схема с 2-3 сообщениями и сервисом с unary + streaming методами, C++ сервер и клиент, CMake-сборка через vcpkg, базовая обработка ошибок и TLS (опционально).

## Ресурсы

- Официальная документация: protobuf.dev (Language Guide, C++ Generated Code, C++ API Reference).
- grpc.io — C++ Quickstart и Basics tutorial.
- Исходники примеров в репозиториях `protocolbuffers/protobuf` и `grpc/grpc` (директории `examples/`).
- `google/protobuf/*.proto` в самом пакете protobuf — читать well-known types как эталонные примеры схем.

## Проверка усвоения

После каждого дня — короткая практическая задача (уже встроена в план). В конце: код-ревью своего итогового проекта на день 10 — проверить совместимость схемы, отсутствие утечек памяти (valgrind/asan), корректную обработку ошибок gRPC.