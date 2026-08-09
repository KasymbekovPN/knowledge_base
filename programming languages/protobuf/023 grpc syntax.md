---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

```protobuf
syntax = "proto3";
package myapp;

// Сообщения для примера — минимальные, специально под демонстрацию service
message OrderRequest {
  int64 order_id = 1;
}

message OrderResponse {
  int64 order_id = 1;
  string status = 2;
}

message OrderUpdate {
  int64 order_id = 1;
  string field = 2;
  string value = 3;
}

message Ack {
  bool ok = 1;
}

service OrderService {
  // 1. Unary — один запрос, один ответ. Самый частый вид RPC,
  //    ведёт себя как обычный вызов функции по сети.
  rpc GetOrder(OrderRequest) returns (OrderResponse);

  // 2. Server streaming — один запрос, СЕРВЕР шлёт поток ответов.
  //    Клиент запросил один раз, сервер отдаёт данные порциями
  //    (например, историю статусов заказа).
  rpc WatchOrderStatus(OrderRequest) returns (stream OrderResponse);

  // 3. Client streaming — КЛИЕНТ шлёт поток запросов, сервер отвечает один раз
  //    в конце (например, серия обновлений полей заказа, применяются пачкой).
  rpc BatchUpdateOrder(stream OrderUpdate) returns (Ack);

  // 4. Bidi (двунаправленный) streaming — оба потока независимы,
  //    клиент и сервер шлют сообщения в любом порядке по одному соединению.
  rpc SyncOrders(stream OrderUpdate) returns (stream OrderResponse);
}
```


**`service`/`rpc` — базовый синтаксис**

```proto
service OrderService {
  rpc MethodName(RequestType) returns (ResponseType);
}
```

Каждый `rpc` компилируется в пару абстрактных методов: клиентский (`StubInterface`) и серверный (`Service`) — реализовывать нужно только серверную сторону, клиентская сторона (`Stub`) генерируется полностью готовой к использованию.

**1. Unary — обычный вызов**

```proto
rpc GetOrder(OrderRequest) returns (OrderResponse);
```

Серверная сигнатура:

```cpp
virtual ::grpc::Status GetOrder(::grpc::ServerContext*, const OrderRequest*, OrderResponse*);
```

Ровно один запрос, ровно один ответ, оба передаются напрямую как указатели — не через поток. Ближе всего к обычному вызову функции по сети, самый частый вид RPC на практике.

**2. Server streaming — `stream` только в `returns`**

```proto
rpc WatchOrderStatus(OrderRequest) returns (stream OrderResponse);
```

```cpp
virtual ::grpc::Status WatchOrderStatus(::grpc::ServerContext*, const OrderRequest*, ::grpc::ServerWriter<OrderResponse>* writer);
```

Запрос — обычный указатель (клиент прислал один раз), а ответ — `ServerWriter<OrderResponse>*`, у которого внутри реализации сервера вызывается `writer->Write(response)` столько раз, сколько нужно, до явного завершения. Подходит, когда клиент один раз спрашивает, а сервер отдаёт данные порциями (история статусов, лог событий, постраничная выдача).

**3. Client streaming — `stream` только у аргумента**

```proto
rpc BatchUpdateOrder(stream OrderUpdate) returns (Ack);
```

```cpp
virtual ::grpc::Status BatchUpdateOrder(::grpc::ServerContext*, ::grpc::ServerReader<OrderUpdate>* reader, Ack* response);
```

Наоборот: `ServerReader<OrderUpdate>*` — сервер вызывает `reader->Read(&update)` в цикле, пока клиент не закончит слать сообщения, и лишь тогда формирует единственный `Ack` в ответ. Подходит для загрузки данных пачками (клиент шлёт много обновлений, сервер подтверждает одним ответом в конце).

**4. Bidi streaming — `stream` у обеих сторон**

```proto
rpc SyncOrders(stream OrderUpdate) returns (stream OrderResponse);
```

```cpp
virtual ::grpc::Status SyncOrders(::grpc::ServerContext*, ::grpc::ServerReaderWriter<OrderResponse, OrderUpdate>* stream);
```

Один объект `ServerReaderWriter<Ответ, Запрос>` совмещает и чтение, и запись — обе стороны шлют сообщения по одному и тому же HTTP/2-соединению независимо друг от друга (можно писать и читать в разных потоках/асинхронно). Самый гибкий, но и самый сложный в реализации вид — например, для live-синхронизации состояния в обе стороны.

**Бонус, который заметил в выводе**

Помимo обычного `Service`, protoc сгенерировал ещё и `StreamedGetOrder`/`StreamedWatchOrderStatus` внутри отдельного `WithStreamedUnaryMethod`-примеса — это часть experimental API grpc для унификации unary-методов под streaming-интерфейс (используется редко, для продвинутых кастомных generic-обработчиков; в обычном коде сервера этого касаться не нужно).
