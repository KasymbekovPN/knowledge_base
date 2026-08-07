---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**`repeated` — списки**

```proto
message Order {
  repeated string item_ids = 1;
  repeated int32 quantities = 2;
}
```

В C++ генерируется `RepeatedField<int32>` / `RepeatedPtrField<string>` — по интерфейсу похоже на вектор (`add_item_ids()`, `item_ids(i)`, `item_ids_size()`). В wire-формате скалярные repeated-поля по умолчанию упаковываются (`packed`) — все значения идут одним length-delimited блоком, а не отдельным tag на каждый элемент, это экономит место.

**`optional` — явное присутствие поля**

В proto3 изначально все скалярные поля имели значение по умолчанию (0, "", false) и не было способа отличить "поле не установлено" от "поле явно равно 0". `optional` (proto3, начиная с версии 3.15) возвращает эту возможность:

```proto
message User {
  optional int32 age = 1;
}
```

Генерируется метод `has_age()`, которого без `optional` не было бы для скалярных типов (для `string`/`message`/`bytes` он есть всегда). Технически `optional` поле реализовано через скрытый `oneof` из одного поля.

**`enum` — перечисления**

```proto
enum Status {
  STATUS_UNKNOWN = 0;
  STATUS_ACTIVE = 1;
  STATUS_BANNED = 2;
}

message User {
  Status status = 1;
}
```

Правило proto3: первое значение enum обязано быть 0 и служит дефолтом. На wire enum кодируется как varint (wire_type 0), как обычный int32. Если сериализатор новой схемы отправит значение enum, которого нет в старой схеме получателя, старый код не упадёт — просто сохранит число как unknown field при повторной сериализации (forward compatibility).

**Вложенные сообщения**

```proto
message Order {
  message Item {
    string sku = 1;
    int32 quantity = 2;
  }
  repeated Item items = 1;
}
```

В C++ это `Order::Item` — вложенный класс. На wire вложенное сообщение — length-delimited поле (wire_type 2): длина + сериализованные байты вложенного сообщения рекурсивно. Можно и ссылаться на сообщение, объявленное в другом файле, через `import`.

**`oneof` — union по смыслу**

```proto
message PaymentMethod {
  oneof method {
    string card_number = 1;
    string paypal_email = 2;
    string crypto_wallet = 3;
  }
}
```

Только одно из полей внутри `oneof` может быть установлено одновременно — установка одного автоматически сбрасывает предыдущее. В C++ это реализовано через union-подобное хранилище (экономия памяти) плюс поле-дискриминатор, генерируется метод `method_case()` для проверки, какое поле активно, и enum `MethodCase`. На wire это просто обычные поля с разными номерами — protobuf гарантирует эксклюзивность только на уровне API, не является отдельным wire-типом.

**`map<K, V>` — словари**

```proto
message Config {
  map<string, int32> settings = 1;
}
```

Генерируется `google::protobuf::Map<std::string, int32>` — интерфейс похож на `std::unordered_map`. Под капотом это синтаксический сахар: на wire `map` разворачивается в `repeated` вложенных сообщений вида:

```proto
message SettingsEntry {
  string key = 1;
  int32 value = 2;
}
repeated SettingsEntry settings = 1;
```

Отсюда ограничение — ключом может быть только integral/string тип (не message, не float), а порядок элементов в map не гарантирован при итерации.
