---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]


**Геттер/сеттер (скаляр и string)**

```cpp
u.set_id(42);        // сеттер
u.id();               // геттер, возвращает по значению для int32
u.set_name("Alice");
u.name();             // геттер, возвращает const std::string&
```

**`mutable_` — доступ "на месте" без копий**

Для string/message/repeated/map полей есть `mutable_X()`, возвращающий указатель/ссылку на внутреннее хранилище — можно модифицировать напрямую, не пересобирая значение снаружи:

```cpp
u.mutable_name()->append(" Smith");   // работает как обычный std::string*
```

Из вывода видно: `"Alice"` → `"Alice Smith"` без промежуточного `set_name(u.name() + " Smith")`.

**`add_` — для repeated message**

```cpp
myapp::Address* addr1 = u.add_addresses();
addr1->set_city("Berlin");
```

`add_addresses()` добавляет новый элемент в конец repeated-поля и возвращает указатель на него — заполняешь прямо через указатель, лишнего копирования нет. Плюс автоматически появляются `addresses_size()` и индексный доступ `addresses(i)` (const-версия) / `mutable_addresses(i)` (мутабельная).

**`has_` — для optional и oneof-полей**

```cpp
addr1->set_apartment("12B");
addr1->has_apartment();  // true (1)
addr2->has_apartment();  // false (0) — apartment не устанавливался
```

Ключевой момент из вывода: `has_apartment()` появляется только потому, что поле объявлено `optional`. Без `optional` в proto3 у скаляров такого метода нет вообще — нельзя отличить "0" от "не установлено".

Для oneof аналог `has_` — это `contact_method_case()`:

```cpp
u.set_phone("+491234567");
u.contact_method_case() == myapp::User::kPhone;  // true

u.set_telegram_handle("@alice");
u.contact_method_case() == myapp::User::kPhone;           // стало false
u.contact_method_case() == myapp::User::kTelegramHandle;  // true
u.phone();  // "" — старое значение обнулилось
```

Это видно прямо в выводе программы: после `set_telegram_handle` поле `phone` автоматически опустело — сеттер одного поля в oneof физически стирает предыдущее.

**`map` — отдельный паттерн, не `add_`, а `mutable_` + operator[]**

```cpp
(*u.mutable_preferences())["theme"] = "dark";
u.preferences().at("theme");  // "dark"
```

### Пример
```cpp
#include <iostream>
#include "user.pb.h"

int main() {
  myapp::User u;

  // --- геттер/сеттер для скаляра ---
  u.set_id(42);
  std::cout << "id = " << u.id() << "\n";

  // --- сеттер/геттер для string ---
  u.set_name("Alice");
  std::cout << "name = " << u.name() << "\n";

  // --- mutable_ для string: изменение "на месте" без промежуточной копии ---
  u.mutable_name()->append(" Smith");
  std::cout << "name after mutable_append = " << u.name() << "\n";

  // --- optional (proto3): has_ появляется только благодаря `optional` ---
  std::cout << "has apartment (Address) before set: n/a — optional только у Address.apartment\n";

  // --- repeated message: add_ возвращает указатель на новый элемент ---
  myapp::Address* addr1 = u.add_addresses();
  addr1->set_city("Berlin");
  addr1->set_street("Alexanderplatz 1");

  myapp::Address* addr2 = u.add_addresses();
  addr2->set_city("Munich");

  std::cout << "addresses_size = " << u.addresses_size() << "\n";
  for (int i = 0; i < u.addresses_size(); ++i) {
    std::cout << "  address[" << i << "].city = " << u.addresses(i).city() << "\n";
  }

  // --- optional на скаляре: has_ метод ---
  addr1->set_apartment("12B");
  std::cout << "addr1.has_apartment() = " << addr1->has_apartment() << "\n";
  std::cout << "addr2.has_apartment() = " << addr2->has_apartment() << " (не установлено)\n";

  // --- map: mutable_ возвращает Map<K,V>&, работаем как с std::map ---
  (*u.mutable_preferences())["theme"] = "dark";
  (*u.mutable_preferences())["locale"] = "en_US";
  std::cout << "preferences[theme] = " << u.preferences().at("theme") << "\n";

  // --- oneof: set_ на одном из полей автоматически "гасит" другое ---
  u.set_phone("+491234567");
  std::cout << "contact_method_case == kPhone? "
            << (u.contact_method_case() == myapp::User::kPhone) << "\n";

  u.set_telegram_handle("@alice");
  std::cout << "after set_telegram_handle:\n";
  std::cout << "  contact_method_case == kPhone? "
            << (u.contact_method_case() == myapp::User::kPhone) << "\n";
  std::cout << "  contact_method_case == kTelegramHandle? "
            << (u.contact_method_case() == myapp::User::kTelegramHandle) << "\n";
  std::cout << "  phone field is now empty: \"" << u.phone() << "\"\n";

  // --- сериализация, чтобы доказать, что объект валиден целиком ---
  std::string bytes;
  u.SerializeToString(&bytes);
  std::cout << "serialized size = " << bytes.size() << " bytes\n";

  myapp::User parsed;
  parsed.ParseFromString(bytes);
  std::cout << "round-trip name = " << parsed.name() << ", addresses = "
            << parsed.addresses_size() << "\n";

  return 0;
}
```
