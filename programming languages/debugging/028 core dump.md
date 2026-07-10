---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]


# Core dumps: генерация и анализ

## Что такое core dump

Core dump — снимок памяти процесса **в момент краха** (segfault, abort, необработанное исключение), записанный на диск. Позволяет отлаживать краш **постфактум**, без необходимости воспроизводить его вживую — критично для крашей, которые случились в проде и не воспроизводятся локально.

## Включение core dump — `ulimit`

По умолчанию **на большинстве систем core dump'ы отключены** (`ulimit -c 0`):Подтверждено: `ulimit -c = 0` — core dump'ы отключены. `core_pattern = core` означает, что при включении файл будет называться просто `core` в рабочей директории процесса.

## Включаем и пишем краш-пример
Отлично, `EXIT: 139` = 128+11 (`SIGSEGV`) — крашится надёжно с первого раза (use-after-free оказался достаточно "злым", чтобы гарантированно попасть на уже размэпленную страницу памяти).

## Генерация core dump

Core dump создан — `561152` байт, ELF-формат, явно помечен `Segmentation fault (core dumped)`. Заметь ключевую фразу в выводе — `(core dumped)` — это подтверждение того, что файл реально записан (без `ulimit -c unlimited` было бы просто `Segmentation fault` без этой пометки).

## Анализ через `gdb ./binary core`
Идеальная посмертная диагностика **без единого запуска отладчика вживую**: `Program terminated with signal SIGSEGV`, точная строка (`crash_demo.cpp:13`, `sum += current->value;`), и — что особенно показательно — `current = 0xc8f5451dc592f45f` — это классический **"poisoned" паттерн** после `delete` (glibc в debug-режимах/некоторых аллокаторах затирает освобождённую память характерным мусором), что само по себе служит уликой use-after-free.

## Анализ через `lldb -c core`
Идентичный результат (`stop reason = signal SIGSEGV: invalid address`, тот же адрес `0xc8f5451dc592f45f`), но LLDB честно указывает точный синтаксис, который использовал (`target create ... --core "core"`) и, как обычно, доходит глубже в backtrace — до `__libc_start_main_impl`, `_start`.

## Ключевые нюансы

### 1. Синтаксис немного отличается между версиями GDB

```bash
gdb ./crash_demo core         # классический синтаксис (позиционные аргументы)
gdb ./crash_demo -c core       # эквивалент через явный флаг
gdb -c core ./crash_demo       # порядок не важен для -c
```

### 2. `core_pattern` определяет имя и МЕСТО файла## Важный практический нюанс — реальные системы часто **не** пишут простой файл `core`

На многих реальных Ubuntu-системах (desktop, некоторые server-образы) `core_pattern` настроен на **пайп** во внешнюю программу:

```
|/usr/share/apport/apport %p %s %c %d %P %E
```

или (более современно, через systemd):

```
|/usr/lib/systemd/systemd-coredump %P %u %g %s %t %c %h
```

В этом случае просто `ulimit -c unlimited` **недостаточно** — core dump либо уйдёт в Apport (Ubuntu desktop crash reporter), либо будет управляться `systemd-coredump`, и достать его нужно уже через:

```bash
coredumpctl list                       # список доступных core dumps под systemd-coredump
coredumpctl gdb <PID_или_имя>            # СРАЗУ открывает GDB на нужном core автоматически
coredumpctl dump <PID> -o /tmp/mycore    # экспортировать core в обычный файл вручную
```

## Docker-специфика (актуально для твоего workflow)

Внутри контейнера `ulimit -c unlimited` работает только если сам **под** (`docker run`) тоже это разрешает:

```bash
docker run --ulimit core=-1 myimage
```

и `core_pattern` внутри контейнера **наследуется от хост-ядра** (это настройка ядра, не namespace-изолированная!) — если на хосте настроен pipe в `systemd-coredump` хоста, а бинарник `systemd-coredump` внутри контейнера отсутствует — core dump может просто потеряться. Надёжнее всего явно задать `core_pattern` на что-то простое перед запуском контейнеров, где важны core dumps:

````bash
echo "core" | sudo tee /proc/sys/kernel/core_pattern    # на ХОСТЕ, не в контейнере
```## Быстрый прогон

```bash
g++ -std=c++20 -g -O0 crash_demo.cpp -o crash_demo
ulimit -c unlimited
./crash_demo                 # упадёт, создаст файл "core" (или core.PID, зависит от core_pattern)

# GDB
gdb ./crash_demo core
(gdb) bt
(gdb) frame 1
(gdb) print head

# LLDB
lldb -c core ./crash_demo
(lldb) bt
(lldb) frame select 1
(lldb) frame variable head
````

## Сводная таблица

|Действие|GDB|LLDB|
|---|---|---|
|Открыть core dump|`gdb ./binary core`|`lldb -c core ./binary`|
|Backtrace на момент краха|`bt`|`bt`|
|Причина краха|`Program terminated with signal SIGSEGV`|`stop reason = signal SIGSEGV`|
|Переключиться на фрейм|`frame N`|`frame select N`|

## Главное отличие core dump от live-отладки

Core dump — это **read-only снимок**: нельзя `continue`, `step`, `next` (программа физически больше не выполняется — процесса нет, есть только его "фотография" памяти на момент смерти). Можно только **исследовать** состояние (переменные, стек, память), но не управлять исполнением. Для этого и существует связка "локально воспроизвёл → нашёл баг вживую" vs "получил core dump из прода → диагностировал постфактум

### Пример

### main.cpp
```cpp
#include <iostream>
#include <vector>

struct Node {
    int value;
    Node* next;
};

int compute_sum(Node* head) {
    int sum = 0;
    Node* current = head;
    while (current != nullptr) {
        sum += current->value;   // <-- здесь случится segfault
        current = current->next;
    }
    return sum;
}

Node* build_broken_list() {
    Node* n1 = new Node{10, nullptr};
    Node* n2 = new Node{20, nullptr};
    n1->next = n2;
    // БАГ: n2->next никогда не установлен в nullptr явно (тут ОК, new
    // не зануляет автоматически - но представим, что где-то есть
    // указатель на уже освобождённую память):
    delete n2;              // n2 освобождён...
    n1->next = n2;           // ...но n1 всё ещё ссылается на него (use-after-free)
    return n1;
}

int main() {
    std::cout << "Building broken list...\n";
    Node* head = build_broken_list();

    std::cout << "Computing sum (краш ожидается здесь)...\n";
    int result = compute_sum(head);   // <-- крашнется внутри

    std::cout << "result = " << result << std::endl;
    return 0;
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
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
    "dependencies": []  
}
```
