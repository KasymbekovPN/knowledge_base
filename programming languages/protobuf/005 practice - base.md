---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]


- **address.proto** — базовые скалярные типы + `optional`. 
- **user.proto** — импортирует `address.proto`, использует вложенный `enum Status`, `repeated Address`, `map<string,string>`, `oneof contact_method` (телефон либо телеграм, не оба сразу). 
- **order.proto** — импортирует оба файла, вложенное `message Item`, вложенный `enum OrderStatus`, ссылки на `User`/`Address` как на message-поля, `repeated Item`, `map`, `optional tracking_number` (появляется только после отправки заказа).

Номера полей нигде не пересекаются внутри одного message и оставлены с запасом (1-11) для будущих полей 12+.Для реальной генерации C++ кода у тебя локально нужен обычный `protoc` (полный, не из grpcio-tools) — команда: `protoc -I. --cpp_out=. address.proto user.proto order.proto`.

Механика:

```proto
import "address.proto";
```

Путь разрешается относительно директорий, переданных protoc через `-I` (он же `--proto_path`). Если файлы лежат в одной папке, достаточно `protoc -I. ...`; если структура сложнее, `-I` можно указывать несколько раз для разных корней.

Типовая раскладка в реальном проекте:

```
proto/
  common/address.proto
  users/user.proto
  orders/order.proto
```

Тогда импорт пишется как путь относительно корня proto-директории:

```proto
import "common/address.proto";
```

и `package` обычно делают многоуровневым, отражая структуру: `package myapp.users;`, `package myapp.orders;` — это уже отдельно от физического расположения файлов, но на практике их часто выравнивают друг с другом.

Три нюанса, которые стоит знать:

**`import public`** — если `b.proto` импортирует `a.proto` через `import public "a.proto";`, то любой, кто импортирует `b.proto`, автоматически получает доступ и к типам из `a.proto`, как будто импортировал его напрямую. Полезно, когда файл переносят и хотят сохранить обратную совместимость для тех, кто импортировал старый путь.

**Циклические импорты не поддерживаются** — `a.proto` не может импортировать `b.proto`, если `b.proto` импортирует `a.proto`. Protoc выдаст ошибку.

**В CMake это автоматизируется** — `protobuf_generate_cpp()` сам резолвит зависимости между .proto файлами в рамках переданного списка, и в сгенерированных `.pb.h` появляются нужные `#include` на файлы, от которых зависит текущий (как в `order.pb.h` был бы `#include "user.pb.h"`).

### common/address.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
option optimize_for = SPEED;  
  
message Address {  
  string street = 1;  
  string city = 2;  
  string postal_code = 3;  
  string country = 4;  
  // не у всех адресов есть квартира/офис  
  optional string apartment = 5;  
}
```

### users/user.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
import "common/address.proto";  
  
option optimize_for = SPEED;  
  
message User {  
  enum Status {  
    STATUS_UNKNOWN = 0;  
    STATUS_ACTIVE = 1;  
    STATUS_SUSPEND = 2;  
    STATUS_DELETED = 3;  
  }  
  
  int32 id = 1;  
  string name = 2;  
  string email = 3;  
  bool is_verified = 4;  
  double balance = 5;  
  bytes avatar = 6;  
  Status status = 7;  
  repeated Address addresses = 8;  
  map<string, string> preferences = 9;  
  
  // ровно один способ связи должен быть указан  
  oneof contact_method {  
    string phone = 10;  
    string telegram_handle = 11;  
  }}
```

### orders/order.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
import "users/user.proto";  
import "common/address.proto";  
  
option optimize_for = SPEED;  
  
message Order {  
  message Item {  
    string sku = 1;  
    string title = 2;  
    int32 quantity = 3;  
    double unit_price = 4;  
  }
    
  enum OrderStatus {  
    ORDER_STATUS_UNKNOWN = 0;  
    ORDER_STATUS_PENDING = 1;  
    ORDER_STATUS_PAID = 2;  
    ORDER_STATUS_SHIPPED = 3;  
    ORDER_STATUS_CANCELLED = 4;  
  }  
  
  int64 order_id = 1;  
  User buyer = 2;  
  Address shipping_address = 3;  
  repeated Item item = 4;  
  OrderStatus status = 5;  
  double total_amount = 6;  
  map<string, string> metadata = 7;  
  // появляется только после отправки  
  optional string tracking_number = 8;  
}
```

```
protoc -I. --cpp_out=. common/address.proto users/user.proto orders/order.proto
```
