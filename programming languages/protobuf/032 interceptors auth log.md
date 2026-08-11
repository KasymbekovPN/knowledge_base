---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

### vcpkg.json
```json
{
  "name": "grpc-cpp-demo",
  "version": "1.0.0",
  "dependencies": [
    "grpc",
    "protobuf"
  ]
}
```

### CMakeLists.txt
```cmake

```

### CMakePresets.json
```json
{
    "version": 6,
    "configurePresets": [
        {
            "name": "base",
            "hidden": true,
            "generator": "Visual Studio 18 2026",
            "architecture": {
                "value": "x64",
                "strategy": "set"
            },
            "binaryDir": "${sourceDir}/build/${presetName}",
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
            "cacheVariables": {
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md",
                "VCPKG_APPLOCAL_DEPS": "OFF"
            }
        },
        {
            "name": "debug",
            "inherits": "base"
        },
        {
            "name": "release",
            "inherits": "base"
        }
    ],
    "buildPresets": [
        {
            "name": "debug",
            "configurePreset": "debug",
            "configuration": "Debug"
        },
        {
            "name": "release",
            "configurePreset": "release",
            "configuration": "Release"
        }
    ]
}
```

### user_service.proto
```protobuf

```

### main.cpp
```cpp

```


---




**Клиентский interceptor — добавление токена + логирование "снаружи"**

```cpp
class ClientAuthLoggingInterceptor : public grpc::experimental::Interceptor {
  void Intercept(InterceptorBatchMethods* methods) override {
    if (methods->QueryInterceptionHookPoint(PRE_SEND_INITIAL_METADATA)) {
      methods->GetSendInitialMetadata()->insert({"authorization", "Bearer secret-token-123"});
    }
    if (methods->QueryInterceptionHookPoint(POST_RECV_STATUS)) {
      // логируем время и статус
    }
    methods->Proceed();  // ключевой момент — без этого RPC зависнет навсегда
  }
};
```

В логе видно: `[client-interceptor] -> GetUser: добавлен auth-токен` — сам код вызова (`client_with_interceptor.cc`) вообще не упоминает токен, он появляется полностью на уровне инфраструктуры канала. Это и есть главная практическая ценность interceptor'ов: убрать повторяющийся boilerplate (auth, логирование, трейсинг) из каждого места вызова в одно центральное место.

**`QueryInterceptionHookPoint` — один `Intercept()` на всё**

Один и тот же метод `Intercept()` вызывается на **разных точках** жизненного цикла RPC — нужно спрашивать `QueryInterceptionHookPoint(...)`, чтобы понять, какая именно точка сейчас (отправка метаданных, отправка сообщения, получение статуса и т.д.), и реагировать только на нужные. `Proceed()` обязателен на **каждый** вызов `Intercept()` — иначе gRPC не продолжит обработку.

**Серверный interceptor — логирование + условная подмена статуса**

```cpp
if (QueryInterceptionHookPoint(POST_RECV_INITIAL_METADATA)) {
  // читаем token из GetRecvInitialMetadata()
}
if (QueryInterceptionHookPoint(PRE_SEND_STATUS)) {
  if (token != "Bearer secret-token-123") {
    methods->ModifySendStatus(Status(UNAUTHENTICATED, "invalid or missing token"));
  }
}
```

Лог сервера показывает оба прогона: с валидным токеном — `токен валиден, статус не трогаем`, с пустым — `подменяю статус на UNAUTHENTICATED`. Клиент без токена реально получил `код=16` (числовое значение `grpc::StatusCode::UNAUTHENTICATED`) вместо данных пользователя.

**Важная честная деталь, которую стоит знать**

В логе видно `[handler] GetUser(user_id=1)` **в обоих** случаях — включая запрос без токена. Это значит: подмена статуса на `PRE_SEND_STATUS` происходит **после** того, как обработчик уже выполнился (он успел вернуть `Status::OK`, но на самом отправлении interceptor переписал итоговый статус). Для чистой логики (например, счётчиков, изменения данных) это может быть неприемлемо — обработчик всё же "поработал". Для настоящего short-circuit до вызова бизнес-логики в реальных системах чаще проверяют токен прямо в начале самого обработчика через `context->client_metadata()`, а interceptor оставляют для сквозного логирования/трейсинга/метрик, где не важно, успел ли отработать handler.

**Регистрация**

```cpp
// клиент
auto channel = grpc::experimental::CreateCustomChannelWithInterceptors(
    address, creds, args, std::move(client_interceptor_creators));

// сервер
builder.experimental().SetInterceptorCreators(std::move(server_interceptor_creators));
```

Обрати внимание на `experimental` в названиях — в grpc 1.30 (версия из этой песочницы) interceptor API официально помечен экспериментальным (хотя стабилен и широко используется на практике); в более новых версиях gRPC он давно стабилизирован и живёт без этого namespace-префикса.




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
- [x] Синхронный сервер/клиент на C++: `grpc::Server`, `ServerBuilder`, `ClientContext`, `Stub`. (2026.08.10)
- [x] Практика: реализовать unary RPC (например, `GetUser(UserRequest) -> UserResponse`), поднять сервер и клиент локально. (2026.08.10)

## День 8: gRPC — streaming и асинхронность

- [x] Server streaming и client streaming на практике (например, стрим логов). (2026.08.10)
- [x] Bidi streaming. (2026.08.11)
- [x] Асинхронный API (`CompletionQueue`) — на уровне понимания, без глубокого погружения. (2026.08.11)
- [x] Обработка ошибок: `grpc::Status`, коды ошибок, deadlines/timeouts, retry-политики. (2026.08.11)
- [x] Практика: добавить server-streaming метод. (2026.08.11)

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