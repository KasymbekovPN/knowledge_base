---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**`DebugString()`**

Печатает сообщение в текстовом формате (по сути text-format protobuf — тот же формат, что читает/пишет `TextFormat::PrintToString`). Экранирует все байты вне ASCII как восьмеричные escape-последовательности: `"Алиса"` превратилось в `"\320\220\320\273\320\270\321\201\320\260"` — это UTF-8 байты кириллицы, показанные побайтово.

**`Utf8DebugString()`**

Тот же формат, но не экранирует валидный UTF-8 — печатает `"Алиса"` как есть, читаемо. Логичный выбор, если знаешь, что строки в сообщении — валидный текст (не произвольные `bytes`), и хочешь читаемый лог.

**`ShortDebugString()`**

Тот же контент, но в одну строку без переносов — удобно для однострочных лог-записей (`LOG(INFO) << msg.ShortDebugString()`).

**Практическое замечание**

Все три метода — только для отладки/логов, не для сериализации в проде: формат текстовый, не совместим по эффективности и стабильности с бинарным wire-форматом, и медленнее. Для машинного обмена данными — всегда `SerializeToString`/`ParseFromString` из прошлой темы.

Ещё нюанс: `bytes`-поля (не `string`) при печати через `DebugString()` тоже экранируются восьмеричными последовательностями всегда — там нет понятия "валидный UTF-8", поэтому `Utf8DebugString()` на них ничего не меняет.

```cpp
#include <iostream>
#include "user.pb.h"

int main() {
  myapp::User u;
  u.set_id(7);
  u.set_name("Алиса");  // не-ASCII, чтобы показать разницу Utf8 vs обычного
  u.add_addresses()->set_city("Berlin");
  (*u.mutable_preferences())["theme"] = "dark";
  u.set_telegram_handle("@alice");

  std::cout << "=== DebugString() ===\n";
  std::cout << u.DebugString();

  std::cout << "\n=== Utf8DebugString() ===\n";
  std::cout << u.Utf8DebugString();

  std::cout << "\n=== ShortDebugString() (однострочный вариант) ===\n";
  std::cout << u.ShortDebugString() << "\n";

  return 0;
}
```
