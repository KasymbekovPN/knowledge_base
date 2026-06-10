---
tags:
  - programming-language
  - cpp
  - threads
---
[[raw data/cpp/os/threads/_|<=]]

Гарантирует, что функция будет вызвана **ровно один раз**, даже если несколько потоков вызывают `call_once` одновременно. Остальные потоки блокируются до завершения первого вызова.

Требует `std::once_flag` — флаг, хранящий состояние "было ли уже выполнено".

Главный use-case — ленивая инициализация синглтона

Если вызванная функция бросила исключение — флаг **не помечается как выполненный**, и следующий поток попробует снова.

`call_once` нужен, когда инициализацию нужно вынести из функции или управлять ею явно.

### Сравнение с альтернативами

| Способ                                | Потокобезопасность | Примечание                              |
| ------------------------------------- | ------------------ | --------------------------------------- |
| `call_once`                           | да                 | стандартный, рекомендуемый              |
| `static` локальная переменная (C++11) | да                 | инициализация гарантированно однократна |
| `double-checked locking` вручную      | сложно             | легко допустить UB                      |

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>

struct Database {
public:
    static Database& instance() {
        std::call_once(init_flag, []() {
            db = std::make_unique<Database>(Database());
        });
        return *db;
    }

    void query() {
        std::cout << "Query execution..." << std::endl;
    }

private:
    Database() {
        std::cout << "DB connected" << std::endl;
    }

    static std::once_flag init_flag;
    static std::unique_ptr<Database> db;
};

std::once_flag Database::init_flag;
std::unique_ptr<Database> Database::db;

void call_worker() {
    Database::instance().query();
}

int main() {
    std::thread t0 {call_worker};
    std::thread t1 {call_worker};

    t0.join();
    t1.join();

    return 0;
}
```

```
DB connected
Query execution...Query execution...
```
