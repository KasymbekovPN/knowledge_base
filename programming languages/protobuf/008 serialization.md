---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]


**`SerializeToString(std::string* output)`**

```cpp
std::string bytes;
original.SerializeToString(&bytes);  // 28 байт для нашего User
```

Самый частый способ — сериализует в `std::string`, которую дальше можно отправить по сети, положить в очередь, записать куда угодно. Возвращает `bool` (успех/неуспех), хотя на практике почти всегда `true`, если само сообщение валидно (нет незаполненных required-полей — но в proto3 required вообще нет).

**`ParseFromString(const std::string& data)`**

```cpp
myapp::User from_string;
from_string.ParseFromString(bytes);  // ok=1, name=Alice
```

Обратная операция. Важно: `ParseFromString` заменяет содержимое сообщения — если нужно домержить данные в уже заполненный объект, есть отдельный `MergeFromString`.

Из вывода видно ключевой момент безопасности — на усечённых данных (`bytes.substr(0, bytes.size()/2)`) `ParseFromString` вернул `ok=0`. Всегда проверяй возвращаемое значение, а не молча доверяй результату.

**`SerializeToOstream(std::ostream* output)` / `ParseFromIstream(std::istream* input)`**

```cpp
std::ofstream out("/tmp/user.bin", std::ios::binary);
original.SerializeToOstream(&out);

std::ifstream in("/tmp/user.bin", std::ios::binary);
myapp::User from_file;
from_file.ParseFromIstream(&in);
```

Для работы с файлами/сокетами напрямую, без промежуточного `std::string`. Обязательно открывать поток в бинарном режиме (`std::ios::binary`) — иначе на Windows возможна порча байтов из-за трансляции `\n`↔`\r\n`.

**`SerializeToArray(void* data, int size)` / `ParseFromArray(const void* data, int size)`**

```cpp
int size = original.ByteSizeLong();     // узнаём точный размер заранее
std::vector<uint8_t> buf(size);
original.SerializeToArray(buf.data(), buf.size());
```

Пишет в уже выделенный буфер фиксированного размера — нет лишней аллокации внутри protobuf, полезно для zero-copy сценариев (сетевые буферы, shared memory, arena-подобные схемы). Обязательно сначала узнать размер через `ByteSizeLong()`, иначе буфер может оказаться мал. В демо видно: буфер в 2 байта дал `ok=0` — метод честно проверяет границы и не пишет за пределы.

**Практическое правило выбора**

- `String` — дефолт для 90% случаев (gRPC, очереди сообщений, HTTP body).
- `Ostream`/`Istream` — для файлов и когда данные и так текут через `std::iostream`-based API.
- `Array` — когда нужен контроль над буфером (высокая производительность, отсутствие лишних аллокаций).

### demo.cpp
```cpp
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include "user.pb.h"

int main() {
  myapp::User original;
  original.set_id(7);
  original.set_name("Alice");
  original.add_addresses()->set_city("Berlin");
  original.set_phone("+491234");

  // --- 1. SerializeToString / ParseFromString ---
  std::string bytes;
  original.SerializeToString(&bytes);
  std::cout << "[String] serialized size = " << bytes.size() << " bytes\n";

  myapp::User from_string;
  bool ok1 = from_string.ParseFromString(bytes);
  std::cout << "[String] ParseFromString ok=" << ok1
            << " name=" << from_string.name() << "\n\n";

  // --- 2. SerializeToOstream / ParseFromIstream (файл) ---
  {
    std::ofstream out("/tmp/user.bin", std::ios::binary);
    bool ok = original.SerializeToOstream(&out);
    std::cout << "[Ostream] SerializeToOstream ok=" << ok << "\n";
  }
  {
    std::ifstream in("/tmp/user.bin", std::ios::binary);
    myapp::User from_file;
    bool ok = from_file.ParseFromIstream(&in);
    std::cout << "[Istream] ParseFromIstream ok=" << ok
               << " name=" << from_file.name() << "\n\n";
  }

  // --- 3. SerializeToArray / ParseFromArray (сырой буфер фиксированного размера) ---
  int size = original.ByteSizeLong();
  std::vector<uint8_t> buf(size);
  bool ok2 = original.SerializeToArray(buf.data(), buf.size());
  std::cout << "[Array] SerializeToArray ok=" << ok2
             << " ByteSizeLong=" << size << "\n";

  myapp::User from_array;
  bool ok3 = from_array.ParseFromArray(buf.data(), buf.size());
  std::cout << "[Array] ParseFromArray ok=" << ok3
             << " name=" << from_array.name() << "\n\n";

  // --- 4. Ошибка: буфер меньше, чем нужно ---
  std::vector<uint8_t> small_buf(2);  // заведомо мало
  bool ok4 = original.SerializeToArray(small_buf.data(), small_buf.size());
  std::cout << "[Array] SerializeToArray into too-small buffer ok=" << ok4
             << " (ожидаем false)\n\n";

  // --- 5. Битые/усечённые данные при парсинге ---
  std::string truncated = bytes.substr(0, bytes.size() / 2);
  myapp::User broken;
  bool ok5 = broken.ParseFromString(truncated);
  std::cout << "[String] ParseFromString(truncated) ok=" << ok5
             << " (ожидаем false, т.к. данные обрублены)\n";

  return 0;
}
```
