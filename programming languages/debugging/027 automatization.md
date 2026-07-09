---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# Полноценная автоматизация: Python API в GDB и Python scripting в LLDB

## Три уровня автоматизации, которые мы разберём

1. **Smart breakpoints** — `gdb.Breakpoint` с переопределённым `stop()` — Python-логика вместо простых expression-условий
2. **Кастомные команды** — `gdb.Command`/`command script add` — своя команда, доступная как `(gdb) dump-threads`
3. **Event hooks** — реакция на события (`gdb.events.stop`, `SBTarget` breakpoint callbacks) без ручного вмешательства

## 1. GDB: Smart Breakpoint — логика на Python вместо expression

Обычное условие (`break X if y > 5`) ограничено простыми expression. `gdb.Breakpoint.stop()` может делать **что угодно** — обходить контейнеры, вести счётчики между остановками, логировать в файл.

## 2. LLDB: аналог того же — кастомные команды и breakpoint-callback'и
Работает без единой проблемы с кодировкой (LLDB, судя по всему, использует Python 3 `sys.stdout` иначе, чем GDB, и не спотыкается на локали) — кастомная команда `dump-threads` сразу показывает функцию каждого потока и значение `id` там, где оно доступно.

```bash
# ВАЖНО: если работаешь в окружении с бедной локалью (как эта песочница) -
# явно задай UTF-8 для GDB, иначе кириллица в Python-выводе упадёт с ошибкой:
export LC_ALL=C.UTF-8

# GDB
gdb -x gdb_automation.py ./threads_demo
(gdb) dump-threads id
(gdb) auto-log-loop main.cpp:26 i 10
(gdb) python EveryNthBreakpoint('main.cpp:26', 5)

# LLDB
lldb ./threads_demo
(lldb) command script import lldb_automation.py
(lldb) dump-threads id
(lldb) breakpoint set --file main.cpp --line 26
(lldb) breakpoint command add 1 -F lldb_automation.every_nth_callback
```

## Итоговая сводная таблица: GDB Python API vs LLDB Python API

|Возможность|GDB|LLDB|
|---|---|---|
|Кастомная команда|`class X(gdb.Command)`, `super().__init__(name, gdb.COMMAND_USER)`|`def func(debugger, command, result, internal_dict)` + `command script add -f module.func name`|
|Smart breakpoint с состоянием|`class X(gdb.Breakpoint)`, override `stop()`|Callback-функция + module-level переменная для состояния (не полагайся на `internal_dict`!)|
|Автозагрузка при импорте|нет отдельного хука — весь top-level код исполняется при `source`|`__lldb_init_module(debugger, internal_dict)` — специальная функция, вызывается автоматически|
|Доступ к потокам|`gdb.selected_inferior().threads()`|`process` (итерируемый) через `debugger.GetSelectedTarget().GetProcess()`|
|Чтение переменной из фрейма|`frame.read_var(name)` — кидает `gdb.error` **или** `ValueError`|`frame.FindVariable(name)` — возвращает "invalid" `SBValue`, проверяй `.IsValid()` вместо try/except|
|Персистентное состояние между вызовами|Естественно — `self.hit_counter` в объекте `gdb.Breakpoint`|Ненадёжно через `internal_dict` — используй module-level переменную|

### Пример

### args evn demo/main.cpp
```cpp
#include <iostream>  
#include <cstdlib>  
#include <iostream>  
#include <format>  
#include <string>  
#include <thread>  
  
int main(int argc, char *argv[]) {  
    // --- Аргументы командной строки ---  
    std::cout << "argc = " << argc << "\n";  
    for (int i = 0; i < argc; ++i) {  
        std::cout << "argv[" << i << "] = " << argv[i] << "\n";   // <-- breakpoint здесь  
    }  
  
    // --- Переменные окружения ---  
    const char* log_level = std::getenv("LOG_LEVEL");  
    const char* asio_disable_threads = std::getenv("BOOST_ASIO_DISABLE_THREADS");  
  
    std::string log_level_str = log_level ? log_level : "(NOT SET)";  
    std::string threads_str = asio_disable_threads ? asio_disable_threads : "(NOT SET)";  
  
    std::cout << "LOG_LEVEL = " << log_level_str << "\n";  
    std::cout << "BOOST_ASIO_DISABLE_THREADS = " << threads_str << "\n";  // <-- и здесь  
  
    // Держим процесс живым, чтобы успеть attach'иться gdb -p / lldb -p    for (int i = 0; i < 300; ++i) {  
        std::cout << "tick " << i << "\n";  
        std::this_thread::sleep_for(std::chrono::seconds(1));  // <-- breakpoint после attach сюда  
    }  
  
    return 0;  
}  
  
/*  
  
###  
Get-Process app | Select-Object Id  
lldb -p 12345  
command script import lldb_automation.py  
dump-threads id  
breakpoint set --file main.cpp --line 28  
breakpoint command add 1 -F lldb_automation.every_nth_callback  
  
###  
Get-Process app | Select-Object Id  
gdb -p 12345  
(gdb) dump-threads id  
(gdb) auto-log-loop main.cpp:26 i 10  
(gdb) python EveryNthBreakpoint('main.cpp:28', 5)  
  
 */
```

### threads demo/main.cpp
```cpp
#include <atomic>  
#include <chrono>  
#include <iostream>  
#include <mutex>  
#include <thread>  
#include <vector>  
  
std::mutex cout_mutex;  
std::atomic<int> shared_counter{0};  
  
void worker(int id, int iterations) {  
    for (int i = 0; i < iterations; ++i) {  
        int local_value = id * 100 + i;   // <-- breakpoint здесь, разный для каждого потока  
        shared_counter.fetch_add(1);  
  
        {            std::lock_guard<std::mutex> lock(cout_mutex);  
            std::cout << "worker " << id << ": iteration " << i  
                      << ", local_value=" << local_value << "\n";  
        }  
        std::this_thread::sleep_for(std::chrono::milliseconds(200));  
    }}  
  
int main() {  
    std::vector<std::thread> workers;  
    const int num_workers = 4;  
  
    for (int id = 0; id < num_workers; ++id) {  
        workers.emplace_back(worker, id, 10);  
    }  
    for (auto& t : workers) {  
        t.join();  
    }  
    std::cout << "Total counter = " << shared_counter.load() << std::endl;  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
  
  
  
###  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
  
 */
```

### gdb_automation.py
```python
"""  
GDB smart breakpoints и автоматизация.  
Загрузка: (gdb) source gdb_automation.py  
"""  
import gdb  
  
class EveryNthBreakpoint(gdb.Breakpoint):  
    """  
    Останавливается только на КАЖДОМ N-ом попадании, остальные - тихо    пропускает и продолжает сам. Логика с состоянием, которую нельзя    выразить обычным expression-условием (нужен персистентный счётчик).    """  
    def __init__(self, location, every_n):  
        super().__init__(location)  
        self.every_n = every_n  
        self.hit_counter = 0  
  
    def stop(self):  
        self.hit_counter += 1  
        if self.hit_counter % self.every_n == 0:  
            gdb.write(f"[EveryNthBreakpoint] Остановка на попадании #{self.hit_counter}\n")  
            return True  # реально остановить выполнение  
        return False  # тихо продолжить, не останавливаясь  
  
  
class ThreadStateBreakpoint(gdb.Breakpoint):  
    """  
    Останавливается, только если СРЕДИ ВСЕХ потоков есть хотя бы один    с конкретным значением локальной переменной. Такую кросс-тредовую    проверку невозможно выразить в обычном breakpoint condition,    который видит только состояние текущего потока.    """  
    def __init__(self, location, var_name, target_value):  
        super().__init__(location)  
        self.var_name = var_name  
        self.target_value = target_value  
  
    def stop(self):  
        inferior = gdb.selected_inferior()  
        for thread in inferior.threads():  
            thread.switch()  
            try:  
                frame = gdb.selected_frame()  
                val = frame.read_var(self.var_name)  
                if int(val) == self.target_value:  
                    gdb.write(f"[ThreadStateBreakpoint] Найдено {self.var_name}="  
                              f"{self.target_value} в потоке {thread.num}\n")  
                    return True  
            except (gdb.error, ValueError):  
                continue  
        return False  
  
class DumpAllThreads(gdb.Command):  
    """  
    Кастомная команда: (gdb) dump-threads    Показывает компактную сводку по всем потокам: номер, функция,    строка, и (если есть в скоупе) значение указанной переменной.    """  
    def __init__(self):  
        super().__init__("dump-threads", gdb.COMMAND_USER)  
  
    def invoke(self, arg, from_tty):  
        var_name = arg.strip() if arg else None  
        inferior = gdb.selected_inferior()  
        current = gdb.selected_thread()  
  
        gdb.write(f"{'ID':<4}{'LWP':<10}{'Function':<30}{'Extra':<20}\n")  
        gdb.write("-" * 64 + "\n")  
  
        for thread in inferior.threads():  
            thread.switch()  
            frame = gdb.selected_frame()  
            func_name = frame.name() or "??"  
            extra = ""  
            if var_name:  
                try:  
                    val = frame.read_var(var_name)  
                    extra = f"{var_name}={val}"  
                except (gdb.error, ValueError):  
                    extra = f"{var_name}=<нет в скоупе>"  
            gdb.write(f"{thread.num:<4}{thread.ptid[1]:<10}{func_name:<30}{extra:<20}\n")  
  
        current.switch()  # вернуться на исходный поток  
  
  
class AutoLogAndContinue(gdb.Command):  
    """  
    (gdb) auto-log-loop <file:line> <count>    Автоматически ставит breakpoint, логирует указанную переменную    и продолжает N раз подряд - замена ручного commands/continue.    """  
    def __init__(self):  
        super().__init__("auto-log-loop", gdb.COMMAND_USER)  
  
    def invoke(self, arg, from_tty):  
        parts = arg.split()  
        if len(parts) < 3:  
            gdb.write("Использование: auto-log-loop <file:line> <var> <count>\n")  
            return  
  
        location, var_name, count = parts[0], parts[1], int(parts[2])  
        bp = gdb.Breakpoint(location)  
  
        if gdb.selected_inferior().pid == 0:  
            gdb.execute("run", to_string=True)  
        else:  
            gdb.execute("continue", to_string=True)  
  
        for i in range(count):  
            if not gdb.selected_thread() or not gdb.selected_thread().is_valid():  
                gdb.write("Процесс завершился раньше времени\n")  
                break  
            try:  
                val = gdb.selected_frame().read_var(var_name)  
                gdb.write(f"[{i+1}/{count}] {var_name} = {val}\n")  
            except gdb.error as e:  
                gdb.write(f"[{i+1}/{count}] ошибка чтения {var_name}: {e}\n")  
  
            if i < count - 1:  
                gdb.execute("continue", to_string=True)  
  
        bp.delete()  
  
  
DumpAllThreads()  
AutoLogAndContinue()
```

### lldp_automation.py
```python
"""  
LLDB scripting: кастомные команды и breakpoint callback'и.  
  
Загрузка:  
  (lldb) command script import lldb_automation.py"""  
import lldb  
  
  
def dump_threads(debugger, command, result, internal_dict):  
    """  
    (lldb) dump-threads [var_name]    Аналог GDB-команды dump-threads: компактная сводка по всем потокам.    """    var_name = command.strip() if command.strip() else None  
    target = debugger.GetSelectedTarget()  
    process = target.GetProcess()  
  
    result.AppendMessage(f"{'ID':<4}{'TID':<10}{'Function':<30}{'Extra':<20}")  
    result.AppendMessage("-" * 64)  
  
    for thread in process:  
        frame = thread.GetFrameAtIndex(0)  
        func_name = frame.GetFunctionName() or "??"  
        extra = ""  
        if var_name:  
            val = frame.FindVariable(var_name)  
            if val.IsValid():  
                extra = f"{var_name}={val.GetValue()}"  
            else:  
                extra = f"{var_name}=<нет в скоупе>"  
        result.AppendMessage(f"{thread.GetIndexID():<4}{thread.GetThreadID():<10}"  
                             f"{func_name:<30}{extra:<20}")  
  
  
_every_nth_state = {"counter": 0}  
  
  
def every_nth_callback(frame, bp_loc, internal_dict):  
    """  
    Breakpoint callback - LLDB-аналог gdb.Breakpoint.stop().    Состояние храним в module-level словаре _every_nth_state, а не в    internal_dict аргументе - на практике internal_dict оказался    ненадёжным для персистентности между вызовами.  
    Подключение:      (lldb) breakpoint set --file main.cpp --line 26      (lldb) breakpoint command add 1 -F lldb_automation.every_nth_callback    Вернуть True - остановиться, False - тихо продолжить (аналог stop()).    """    _every_nth_state["counter"] += 1  
    counter = _every_nth_state["counter"]  
  
    every_n = 5  
    if counter % every_n == 0:  
        print(f"[every_nth_callback] Остановка на попадании #{counter}")  
        return True  
    return False  
  
def __lldb_init_module(debugger, internal_dict):  
    """  
    Вызывается автоматически при 'command script import' - здесь    регистрируем кастомные команды, аналог автозагрузки в GDB.    """    debugger.HandleCommand(  
        'command script add -f lldb_automation.dump_threads dump-threads'  
    )  
    print("lldb_automation.py loaded: dump-threads command available")
```

### [both]/CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(Demo LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)
```

### [both]/CMakePresets.json
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

### [both]/vcpkg.json
```json
{  
    "name": "demo",  
    "version": "0.1.0",  
    "dependencies": []  
}
```
