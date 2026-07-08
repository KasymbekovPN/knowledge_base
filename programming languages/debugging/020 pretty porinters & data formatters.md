---
tags:
  - programming-language
  - debug
  - gdb
  - lldb
---
[[programming languages/debugging/_|<=]]

# STL-контейнеры: pretty-printers (GDB) vs data formatters (LLDB)

## Суть проблемы, которую решают оба механизма

Без pretty-printers `print` для `std::vector<int>` показал бы сырую внутреннюю структуру — три указателя (`_M_start`, `_M_finish`, `_M_end_of_storage`), а не человекочитаемый список элементов. Pretty-printers/data formatters — это **слой поверх** сырого GDB/LLDB, который знает внутренний layout конкретных типов (libstdc++, libc++) и рисует их красиво.

## Сводная таблица результатов

|Тип|GDB|LLDB|
|---|---|---|
|`std::vector<int>`|✅ `{10, 20, 30, 40, 50}`|✅ `size=5 {[0]=10, ...}`|
|`std::vector<std::string>`|✅|✅ (с DWARF-warning, но работает)|
|`std::map`|✅ `{["alice"]=95, ...}`|✅ `{[0]=(first="alice", second=95), ...}`|
|`std::unique_ptr` (сам объект)|✅|✅ показывает адрес|
|`*unique_ptr` (разыменование)|✅ `{name=..., priority=...}`|❌ ошибка JIT-вызова `operator*`|
|`std::shared_ptr` (сам объект, показывает значение)|✅|✅|
|`std::optional`|✅ `[contained value]`/`[no contained value]`|✅ `Has Value=true/false`|
|`std::variant`|✅ `[index N]`|✅ `Active Type = ...`|

## Пример

### main.cpp
```cpp
!!!
```

### task_formatter.py
```python
!!!
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20)  
project(proj LANGUAGES CXX)  
  
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

----



## Как посмотреть содержимое `unique_ptr` в LLDB, раз обычный путь не работает

Через сырое чтение памяти по внутреннему указателю (видно из вывода `pointer = 0x...`):

```
(lldb) print *(Task*)0x0000555555577430    # подставить реальный адрес из предыдущего вывода
```

Не элегантно, но рабочий обходной путь, если действительно нужно посмотреть содержимое, а не просто адрес.

---

## Python pretty-printer для СВОЕГО типа (GDB)

Раз мы выяснили, что `Task` печатается стандартно как `{name=..., priority=...}` — но что если хочется кастомный, более наглядный формат? Пишем свой Python pretty-printer:Работает — вместо стандартного `{name = "Deploy service", priority = 1}` теперь `Task("Deploy service", priority=HIGH)`, причём **вложенный** внутри `std::vector<Task>` тоже подхватился автоматически. (Двойные кавычки — мелкая деталь: `self.val['name']` уже возвращает `gdb.Value`, который сам оборачивает строку в кавычки при str(); в реальном коде я бы сделал `str(name)[1:-1]`, чтобы убрать дублирование.)## Как автозагрузить свой pretty-printer постоянно (не вручную `source` каждый раз)

**`.gdbinit` в директории проекта:**

```
source task_printer.py
```

```bash
gdb -q -x .gdbinit ./stl_demo
```

## LLDB — аналог через Python-скрипт (кратко, углубимся на Этапе 5)

LLDB тоже поддерживает кастомные Python-форматтеры, но синтаксис принципиально другой (через `summary`, а не `to_string()`):

```python
def task_summary(valobj, internal_dict):
    name = valobj.GetChildMemberWithName('name').GetSummary()
    priority = valobj.GetChildMemberWithName('priority').GetValueAsSigned()
    return f'Task({name}, priority={priority})'
```

```
(lldb) command script import task_formatter.py
(lldb) type summary add Task -F task_formatter.task_summary
```

Полноценно этим займёмся на **Этапе 5** плана (Python API в GDB и Python scripting в LLDB) — там будет смысл разобрать оба подхода параллельно и глубже.

---

## Итоговая сводка по разделу

| |GDB|LLDB|
|---|---|---|
|Механизм|Python pretty-printers, регистрируются через `gdb.pretty_printers`|Python summary/synthetic providers через `type summary add`|
|STL из коробки|✅ через auto-load `.debug_gdb_scripts`|✅ встроено в сам LLDB (data formatters)|
|`unique_ptr` разыменование|✅ надёжно|⚠️ известная проблема на Linux/libstdc++|
|Кастомный тип|Пишешь `.py`, `source` или auto-load|Пишешь `.py`, `command script import`|


---

```cpp
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

struct Task {
    std::string name;
    int priority;
};

int main() {
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    std::vector<std::string> names = {"Pablo", "Anna", "Max"};
    std::map<std::string, int> scores = {{"alice", 95}, {"bob", 87}, {"carol", 92}};
    std::unique_ptr<Task> task = std::make_unique<Task>(Task{"Deploy service", 1});
    std::shared_ptr<int> shared_counter = std::make_shared<int>(42);
    std::optional<int> maybe_value = 7;
    std::optional<int> empty_value = std::nullopt;
    std::variant<int, std::string> var_value = "hello variant";
    std::vector<Task> tasks = {{"Build", 2}, {"Test", 1}, {"Deploy", 3}};

    std::cout << "numbers.size() = " << numbers.size() << "\n";  // <-- breakpoint здесь
    std::cout << "scores[\"bob\"] = " << scores["bob"] << "\n";
    return 0;
}
```


```python
import gdb

class TaskPrinter:
    """Pretty-printer для struct Task { std::string name; int priority; }"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        name = self.val['name']
        priority = int(self.val['priority'])
        priority_label = {1: "HIGH", 2: "MEDIUM", 3: "LOW"}.get(priority, "UNKNOWN")
        return f'Task("{name}", priority={priority_label})'


def task_lookup(val):
    if str(val.type.strip_typedefs()) == "Task":
        return TaskPrinter(val)
    return None


gdb.pretty_printers.append(task_lookup)
```

----
---
---
----
----
---
---
---
---
----
---


## Этап 3. Исследование состояния программы (2-3 дня)

- Работа с STL-контейнерами — pretty-printers для GDB (Python-based) и встроенная поддержка LLDB (data formatters)

## Этап 4. Отладка многопоточного и асинхронного кода (3-4 дня) — _с прицелом на твой опыт с Asio/корутинами_

- Команды для потоков: `info threads`, `thread apply all bt`, переключение потоков
- Non-stop mode и scheduler-locking в GDB
- Отладка гонок данных, deadlock через анализ стеков всех потоков
- Особенности отладки корутин C++20: как читать состояние `promise_type`, проблемы с оптимизированными фреймами корутин, символы для coroutine frame
- Отладка callback-based кода Boost.Asio: точки останова внутри lambda, работа с io_context

## Этап 5. Python API в GDB и Python scripting в LLDB (2-3 дня)

- Написание кастомных pretty-printers для своих классов (GDB Python API)
- Автоматизация: скрипты для повторяющихся сценариев отладки
- LLDB Python scripting bridge (`script`, кастомные команды)
- Написание `.gdbinit` / `.lldbinit` с полезными алиасами и автозагрузкой printers

## Этап 6. Продвинутая отладка (3-4 дня)

- Core dumps: генерация, анализ (`gdb ./binary core`, `lldb -c core`)
- Reverse debugging в GDB (`record`, `reverse-step`, `reverse-continue`)
- Отладка оптимизированного кода (`-O2`), проблемы с инлайнингом и переменными "optimized out"
- ASan/UBSan интеграция с отладчиком, анализ crash-репортов
- Отладка shared libraries, символы (`.debug` файлы, `objcopy --only-keep-debug`, символьные серверы)
- Remote debugging (`gdbserver`, `lldb-server`) — актуально для отладки внутри Docker/Alpine контейнеров

## Этап 7. Интеграция с инструментами (1-2 дня)

- GDB/LLDB внутри VS Code и CLion — конфигурация `launch.json`, интеграция с CMake presets
- `rr` (record & replay) как надстройка над GDB для детерминированной отладки
- TUI-режим GDB (`gdb -tui`) и его аналоги

---

## Формат работы

Как и в прошлых темах (CMake, Boost, корутины) — предлагаю двигаться блоками с практическими примерами: на каждом этапе будем брать конкретный кусок кода (например, твой `socketbuf` или producer/consumer с корутинами) и разбирать его отладку вживую.

Хочешь начать с Этапа 1, или есть тема, с которой хочешь стартовать сразу (например, отладка корутин — раз ты недавно ей глубоко занимался)?