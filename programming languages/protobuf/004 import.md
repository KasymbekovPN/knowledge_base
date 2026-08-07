---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**`package` — пространство имён**

```proto
syntax = "proto3";
package myapp.orders;

message Order { ... }
```

Определяет namespace для сообщений, чтобы избежать коллизий имён между .proto файлами разных модулей. В C++ генерируется `namespace myapp { namespace orders { ... } }` (в новых версиях protobuf с `option cc_enable_arenas` и совместимости — вложенные namespace через `::`, стиль зависит от версии). Ссылаться на тип из другого пакета можно как `myapp.orders.Order` в .proto или `myapp::orders::Order` в C++.

**Импорты между файлами**

```proto
// user.proto
syntax = "proto3";
package myapp;
message User {
  int32 id = 1;
  string name = 2;
}
```

```proto
// order.proto
syntax = "proto3";
package myapp;

import "user.proto";

message Order {
  User buyer = 1;   // используем сообщение из другого файла
  int32 total = 2;
}
```

При генерации `protoc --cpp_out=. order.proto` компилятор сам подтягивает зависимости — в `order.pb.h` появится `#include "user.pb.h"`. Путь в `import` разрешается относительно `-I`/`--proto_path`, переданного protoc (по умолчанию — текущая директория).

Полезные варианты импорта:

- `import public "foo.proto";` — реэкспортирует содержимое foo.proto тем, кто импортирует уже этот файл (транзитивная видимость).
- `import weak "foo.proto";` — необязательная зависимость, используется редко (в основном внутри Google).

**`option cc_enable_arenas`**

```proto
option cc_enable_arenas = true;
```

Включает поддержку arena allocation в сгенерированном C++ коде для сообщений этого файла (актуально в основном для старых версий protobuf, где arena была опциональной фичей; в современных версиях arena-поддержка встроена по умолчанию, и опция мало на что влияет). Arena — это способ выделять память под все вложенные сообщения одним большим блоком вместо множества отдельных `new`/`delete`, что сильно ускоряет создание/уничтожение больших графов сообщений. Актуально проверить в документации своей версии protobuf, нужна ли эта опция явно.

**`option optimize_for`**

```proto
option optimize_for = SPEED;       // по умолчанию
// или
option optimize_for = CODE_SIZE;
// или
option optimize_for = LITE_RUNTIME;
```

Управляет тем, какой код генерирует protoc:

- `SPEED` (дефолт) — генерируется полный код сериализации/парсинга напрямую, максимальная производительность, но больше размер бинарника.
- `CODE_SIZE` — вместо специализированного кода для каждого сообщения используется общая reflection-based реализация — компактнее, но медленнее на рантайме. Полезно для проектов с сотнями сообщений, где важен размер бинарника (например, мобильные приложения).
- `LITE_RUNTIME` — генерирует код на основе `libprotobuf-lite` вместо полного `libprotobuf`. Убирает reflection API, дескрипторы, `DebugString()` — сильно уменьшает размер рантайм-библиотеки и бинарника, но теряется возможность интроспекции сообщений (не подойдёт, если нужен generic-код через Reflection API или интеграция с чем-то вроде gRPC reflection service).

Для типичного C++ сервиса на практике почти всегда достаточно дефолтного `SPEED`; `LITE_RUNTIME` имеет смысл на embedded/мобильных платформах с жёсткими ограничениями по размеру.
