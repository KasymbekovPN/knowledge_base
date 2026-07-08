---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

## Linux — `gdb -p PID` / `lldb -p PID`

### Базовый синтаксис

```bash
gdb -p 12345
lldb -p 12345
```

или найти PID и прицепиться одной командой:

```bash
gdb -p $(pgrep -f args_env_demo)
lldb -p $(pgrep -f args_env_demo)
```

### Внутри уже запущенной сессии

```bash
(gdb) attach 12345
(lldb) attach --pid 12345
# или по имени процесса:
(lldb) attach --name args_env_demo
```

### Ключевое ограничение Linux: `ptrace_scope`

Начиная с ядра с включённым **Yama LSM** (по умолчанию в Ubuntu), непривилегированный процесс не может attach'иться к чужому процессу без явного разрешения:

```bash
cat /proc/sys/kernel/yama/ptrace_scope
```

Значения:

- **0** — разрешено attach к любому процессу того же пользователя (классическое Unix-поведение)
- **1** — разрешено **только** к прямым child-процессам (дефолт на Ubuntu!)
- **2** — только root
- **3** — полностью запрещено, даже root

**Если ловишь `ptrace: Operation not permitted`** — это первое, что нужно проверить:

```bash
# Временно (до перезагрузки)
sudo sysctl kernel.yama.ptrace_scope=0

# Постоянно
echo "kernel.yama.ptrace_scope = 0" | sudo tee /etc/sysctl.d/10-ptrace.conf
```

Альтернатива без снижения общей защиты — дать capability конкретно GDB:

```bash
sudo setcap cap_sys_ptrace=eip $(which gdb)
```

### Docker-специфика (раз ты активно работаешь с контейнерами)

Внутри контейнера по умолчанию `ptrace` **запрещён** seccomp-профилем Docker, даже с `ptrace_scope=0` на хосте:

```bash
# Разрешить ptrace внутри контейнера
docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined myimage
```

Без этого `gdb -p PID` внутри контейнера выдаст `Operation not permitted`, даже если ты root внутри контейнера — seccomp режет `ptrace()` syscall на уровне ядра до проверки capabilities.

### Отсоединиться, не убивая процесс

```bash
(gdb) detach
(lldb) detach
```

## Windows — attach через GDB/LLDB (MSYS2/MinGW)

### Синтаксис тот же

```bash
gdb -p 12345
lldb -p 12345
```

Найти PID на Windows:

```powershell
# PowerShell
Get-Process args_env_demo | Select-Object Id

# или в cmd
tasklist | findstr args_env_demo
```

### Ключевое отличие от Linux: права администратора

На Windows нет `ptrace_scope`, но есть **debug privilege** (`SeDebugPrivilege`). Attach к процессу другого пользователя (или защищённому системному процессу) требует запуска GDB/LLDB **от имени администратора**:

```
Правая кнопка на MSYS2 UCRT64 → "Запуск от имени администратора"
```

Attach к своему же процессу, запущенному из-под того же пользователя — обычно проходит без admin прав.

### Важный нюанс MinGW GDB на Windows

`gdb -p PID` на Windows работает через **Win32 Debug API** (`DebugActiveProcess`), а не через ptrace — механизм принципиально другой. Из практических последствий:

- Attach иногда работает **медленнее** и менее стабильно, чем на Linux, особенно к процессам со сложным threading
- Некоторые версии MinGW GDB плохо переживают attach к процессу, который уже что-то печатает в консоль (могут быть проблемы с shared console handle) — если ловишь зависание, попробуй запустить целевой процесс в отдельном окне консоли

### LLDB на Windows — хуже поддержан для attach

На практике LLDB под MSYS2/MinGW менее зрелый именно в сценарии attach (в отличие от Linux, где он на равных с GDB) — если что-то не работает, это первое, что стоит списать на известное ограничение, а не на свою ошибку.

## Пример

### main.cpp
```cpp
#include <chrono>  
#include <iostream>  
#include <format>  
#include <thread>  
  
int main(int argc, char *argv[]) {  
    // --- Аргументы командной строки ---  
    std::cout << std::format("argc = {}\n", argc);  
    for (int i{}; i < argc; ++i) {  
        std::cout << std::format("argv[{}] = {}\n", i, argv[i]);  
    }  
    // --- Переменные окружения ---  
    const char* LOG_LEVEL = std::getenv("LOG_LEVEL");  
    const char* ASIO_DOSABLE_THREADS = std::getenv("BOOST_ASIO_DISABLE_THREADS");  
  
    const std::string slog_level = LOG_LEVEL ? LOG_LEVEL : "NOT SET";  
    const std::string sthreads = ASIO_DOSABLE_THREADS ? ASIO_DOSABLE_THREADS : "NOT SET";  
  
    std::cout << std::format("LOG_LEVEL: {}\n", slog_level);  
    std::cout << std::format("BOOST_ASIO_DISABLE_THREADS: {}\n", sthreads);  
  
    // Держим процесс живым, чтобы успеть attach'иться gdb -p / lldb -p  
    for (int i{}; i < 300q; ++i) {  
        std::cout << std::format("tick {}\n", i);  
        std::this_thread::sleep_for(std::chrono::seconds(1));  
    }  
    return 0;  
}  
  
/*  
  
Get-Process app | Select-Object Id  
lldb -p 3420  
(lldb) breakpoint set --file main.cpp --line 26  
(lldb) continue  
(lldb) print i  
  
 */
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(Demo LANGUAGES CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_20)
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
                "CMAKE_CXX_FLAGS_DEBUG": "-g -gdwarf-4 -O0"  
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
    "dependencies": []  
}
```
