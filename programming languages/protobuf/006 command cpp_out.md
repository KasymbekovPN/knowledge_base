---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]


Разбираю реальный код из `user.pb.h`/`user.pb.cc` на нашем примере.**Команда**

```
protoc -I. --cpp_out=gen address.proto user.proto order.proto
```

`-I.` (он же `--proto_path`) задаёт корень, относительно которого резолвятся `import`. `--cpp_out=gen` — куда класть сгенерированные файлы; для каждого `.proto` появляется пара `.pb.h`/`.pb.cc`.


**`.pb.h` — заголовок**

Из сгенерированного `user.pb.h` видно закономерность на каждое поле:

- `string`/`bytes` поля (`name`) — четыре перегрузки `set_name()` (const ref, rvalue, const char*, const char* + size) плюс `name()` (геттер), `mutable_name()` (указатель для in-place модификации), и приватные `_internal_*` версии, которые вызываются извне через публичные обёртки.
- `repeated message` (`addresses`) — `add_addresses()` возвращает указатель на только что добавленный элемент, который сразу можно заполнять.
- `map` (`preferences`) — `preferences()` возвращает const-ref на `Map<K,V>`, `mutable_preferences()` — мутабельную ссылку, `clear_preferences()` — очистка.
- `oneof` (`contact_method`) — генерируется `enum ContactMethodCase`, метод `contact_method_case()` для проверки активного поля, `_internal_has_phone()`/`has_phone()`, приватный `set_has_phone()` для переключения дискриминатора.
- Класс `User` наследуется от `::google::protobuf::Message` (в примере через `PROTOBUF_FINAL` — макрос, отключающий дальнейшее наследование от сгенерированных классов).

**`.pb.cc` — реализация**

Здесь находится:

- Регистрация дескриптора сообщения (метаинформация о полях, нужна для reflection API и `DebugString()`).
- Фактическая логика `SerializeWithCachedSizes`/`MergePartialFromCodedStream` (в старых версиях) или аналог в новых — код, который построчно проходит по полям и пишет/читает их в wire-формате согласно тегам, разобранным в теме про wire-format.
- Реализация `CopyFrom`, `MergeFrom`, `Clear`, деструктора с освобождением памяти под repeated/string/message-поля (или отвязкой от Arena, если она используется).

**На практике из этого API нужно помнить всего 4 паттерна:**

```cpp
User u;
u.set_name("Alice");                    // скаляр/string
u.add_addresses()->set_city("Berlin");  // repeated message
(*u.mutable_preferences())["theme"] = "dark";  // map
u.set_phone("+491234");                 // oneof — автоматически "гасит" telegram_handle
```
