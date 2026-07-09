---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

### Пример

### main.cpp
```cpp
#include <chrono>  
#include <boost/asio.hpp>  
#include <iostream>  
#include <format>  
#include <memory>  
  
class TimerChain: public std::enable_shared_from_this<TimerChain> {  
public:  
    TimerChain(boost::asio::io_context& io, int id):  
        timer_{io},  
        id_{id},  
        tick_count_{0} {}  
  
    void start() { schedule_next(); }  
private:  
    void schedule_next() {  
        timer_.expires_after(std::chrono::milliseconds(100));  
        auto self = shared_from_this();  
        timer_.async_wait([self](const boost::system::error_code& ec) {  
            self->on_timer(ec); // <-- breakpoint  
        });  
    }  
    void on_timer(const boost::system::error_code& ec) {  
        if (ec) {  
            std::cout << std::format("timer {}, error: {}\n", id_, ec.message());  
            return;  
        }  
        ++tick_count_;  
        std::cout << std::format("timer {}, tick: {}\n", id_, tick_count_); // <-- breakpoint  
  
        if (tick_count_ < 3) { schedule_next(); }  
    }  
    boost::asio::steady_timer timer_;  
    int id_;  
    int tick_count_;  
};  
  
int main() {  
    boost::asio::io_context io;  
  
    auto chain_a = std::make_shared<TimerChain>(io, 1);  
    auto chain_b = std::make_shared<TimerChain>(io, 2);  
  
    chain_a->start();  
    chain_b->start();  
  
    std::cout << "io_context.run() starting\n";  
    io.run(); // <-- breakpoint  
    std::cout << "io_context.run() finished, all handlers done\n";  
  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 20  
breakpoint set --file main.cpp --line 31  
breakpoint set --file main.cpp --line 51  
run  
continue  
print self.pointer  
print *(TimerChain*)0x0000029ff7bfff70  
  
  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
break main.cpp:20  
break main.cpp:31  
break main.cpp:51  
run  
  
 */
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(Demo LANGUAGES CXX)  
  
find_package(Boost REQUIRED COMPONENTS system)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)  
  
target_link_libraries(  
        app  
        PRIVATE  
        Boost::system  
        Boost::boost
)
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {
			"name": "base",  
            "hidden": true,  
            "generator": "Ninja",  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"  
        },  
        {
			"name": "debug",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Debug",  
                "CMAKE_CXX_FLAGS_DEBUG": "-g3 -gdwarf-5 -O0"  
            }  
        }
	],
	"buildPresets": [  
        {
			"name": "debug",  
            "configurePreset": "debug"  
        }  
    ]
}
```

### vcpkg.json
```json
{  
    "name": "demo",  
    "version": "0.1.0",  
    "dependencies": ["boost-asio"]  
}
```

Условно только для конкретного объекта:

```
(gdb) break asio_demo.cpp:22 if self->id_ == 2
```

## Сводка практических приёмов для Asio

|Задача|Приём|
|---|---|
|Поймать конкретный handler среди многих|Условный breakpoint по захваченному состоянию (`self->id_ == N`)|
|Понять, откуда вызван handler|`bt` — вся цепочка `scheduler::run → do_run_one → scheduler_operation::complete → wait_handler::do_complete → ...`|
|Убедиться, что все async-операции завершились|Breakpoint **сразу после** `io.run()`, а не попытка вызвать `.stopped()` (может не резолвиться из-за inline)|
|Отследить `shared_ptr` self-ownership в цепочке callback'ов|`print self` — видно `use count`/`weak count` прямо в GDB|

## Практическое замечание для твоего реального кода

Раз ты у себя работаешь с более сложными сценариями (TCP producer/consumer, Strands, несколько потоков в `io_context::run()`) — комбинируй этот подход с темами, которые мы уже разобрали:

- **`scheduler-locking`** — если несколько потоков вызывают `io.run()` одновременно (thread pool), чтобы шагать по одному handler'у не теряя контекст остальных
- **`thread apply all bt`** — чтобы увидеть, какие потоки сейчас реально исполняют handler'ы, а какие простаивают в `epoll_wait`/`scheduler::run`
