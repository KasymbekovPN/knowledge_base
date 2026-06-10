---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

Мьютекс с двумя режимами захвата — позволяет **нескольким читателям** работать одновременно, но **запись монопольна**.

| Режим             | Кто держит | Другие читатели | Другие писатели |
| ----------------- | ---------- | --------------- | --------------- |
| shared (read)     | читатель   | допускаются     | блокируются     |
| exclusive (write) | писатель   | блокируются     | блокируются     |

### Когда использовать

**Выгодно** — если чтений значительно больше, чем записей (кэши, конфиги, справочники).

**Невыгодно** — если запись происходит так же часто, как чтение: `shared_mutex` тяжелее обычного `mutex`, накладные расходы съедают выигрыш.

### RAII-обёртки

- `std::shared_lock` — захват в режиме **чтения** (shared)
- `std::unique_lock` / `std::lock_guard` — захват в режиме **записи** (exclusive)

### shared_timed_mutex

Есть также `std::shared_timed_mutex` — то же самое, но с поддержкой `try_lock_for` / `try_lock_until` с таймаутом, если нужно не блокироваться бесконечно.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <unordered_map>

struct Cache {
public:
    std::string get(const std::string& _key) {
        std::shared_lock lock{mtx};
        auto&& it = data.find(_key);
        return it != data.end() ? it->second : "";
    }

    void set(const std::string &_key,
             const std::string &_value) {
        std::unique_lock lock{mtx};
        data[_key] = _value;
    }

private:
    std::shared_mutex mtx;
    std::unordered_map<std::string, std::string> data;
};

int main(int argc, char const *argv[]) {
    Cache cache;
    cache.set("user", "Alice");

    std::thread t0{[&]() { std::cout << cache.get("user") << std::endl; }};
    std::thread t1{[&]() { std::cout << cache.get("user") << std::endl; }};
    std::thread t2{[&]() { cache.set("user", "Bob"); }};

    t0.join();
    t1.join();
    t2.join();

    return 0;
}
```

```
Alice
Alice
```
