---
tags:
  - programming-language
  - cpp
  - threads
---
[[programming languages/cpp/threads/_|<=]]

## std::jthread (C++20)

Улучшенная версия `std::thread` с двумя ключевыми добавлениями: **автоматический `join` в деструкторе** и **встроенная поддержка отмены** через `stop_token`.

```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "Worker executed" << std::endl;
}

int main() {
    {
        std::thread t{worker};
        t.join();
    }

    {
        std::jthread t{worker};
    }

    return 0;
}
```

```
Worker executed
Worker executed
```

### Отмена через stop_token

`stop_token` передаётся автоматически, если первый параметр функции — `std::stop_token`.

```cpp
#include <iostream>
#include <thread>

void worker(std::stop_token _stoken)  {
    while (!_stoken.stop_requested()) {
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "stopped\n";
}

int main() {
    std::jthread t{worker};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
```

```
working...
working...
working...
working...
working...
stopped
```

### stop_token API

|Метод|На ком вызывается|Описание|
|---|---|---|
|`request_stop()`|`jthread` / `stop_source`|Запросить остановку|
|`get_stop_token()`|`jthread` / `stop_source`|Получить `stop_token`|
|`get_stop_source()`|`jthread`|Получить `stop_source`|
|`stop_requested()`|`stop_token`|Был ли запрос остановки|
|`stop_possible()`|`stop_token`|Связан ли токен с источником|
|`stop_callback{token, fn}`|—|Вызвать `fn` при `request_stop()`|

### stop_callback — реакция на запрос остановки

```cpp
#include <iostream>
#include <thread>

void worker(std::stop_token _stoken) {
    std::stop_callback scallback{
        _stoken,
        []() { std::cout << "stop required!\n"; }
    };

    while (!_stoken.stop_requested()) {
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::jthread t{worker};
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
```

```
working...
working...
working...
working...
working...
stop required!
```

### std::stop_source

`stop_source` позволяет управлять остановкой **извне потока**, не имея прямого доступа к `jthread`.

Один `stop_source` — один `request_stop()` — все потоки получившие его токен останавливаются одновременно.

```cpp
#include <iostream>
#include <thread>

void worker(std::stop_token _stoken, int _id) {
    while (!_stoken.stop_requested()) {
        std::cout << "worker " << _id << " working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "worker " << _id << " stopped\n";
}

int main() {
    std::stop_source source;

    std::jthread t0{worker, source.get_token(), 0};
    std::jthread t1{worker, source.get_token(), 1};

    std::this_thread::sleep_for(std::chrono::seconds(1));
    source.request_stop();

    return 0;
}
```

```
worker 1 working...
worker 0 working...
worker 0 working...
worker 1 working...
worker 1 working...
worker 0 working...
worker 0 working...
worker 1 working...
worker 0 working...
worker 1 working...
worker 0 stopped
worker 1 stopped
```

### condition_variable_any + stop_token

`std::condition_variable_any::wait` умеет принимать `stop_token` — пробуждается и при `notify` и при `request_stop`

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable_any cv;
std::queue<int> queue;

void worker(std::stop_token _stoken) {
    std::cout << "Start\n";
    std::unique_lock lock{mtx};
    cv.wait(lock, _stoken, [](){ return !queue.empty(); });
    std::cout << "Done\n";
}

int main() {
    std::jthread t{worker};
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    return 0;
}
```

```
Start
Done
```

### jthread vs thread

|               | `std::thread` | `std::jthread` |
| ------------- | ------------- | -------------- |
| Auto join     | нет           | да             |
| Отмена        | нет           | `stop_token`   |
| C++ версия    | C++11         | C++20          |
| Совместимость | везде         | только C++20+  |
