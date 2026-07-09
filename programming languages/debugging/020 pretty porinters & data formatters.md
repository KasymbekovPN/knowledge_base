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
#include <iostream>  
#include <format>  
#include <memory>  
#include <optional>  
#include <variant>  
#include <vector>  
#include <map>  
#include <string>  
  
struct Task {  
    std::string name;  
    int priority;  
};  
  
int main() {  
    std::vector<int> numbers = {10, 20, 30, 40, 50};  
    std::vector<std::string> names = {"Pablo", "Anna", "Max"};  
    std::map<std::string, int> scores = {  
        {"alice", 95},  
        {"bob", 87},  
        {"carol", 92}};  
    std::unique_ptr<Task> task = std::make_unique<Task>(  
        Task{"Deploy service", 1});  
    std::shared_ptr<int> shared_counter = std::make_shared<int>(42);  
    std::optional<int> maybe_value = 7;  
    std::optional<int> empty_value = std::nullopt;  
    std::variant<int, std::string> var_value = "hello variant";  
    std::vector<Task> tasks = {  
        {"Build", 2},  
        {"Test", 1},  
        {"Deploy", 3}};  
  
    std::cout << "numbers.size() = " << numbers.size() << "\n";  // <-- breakpoint здесь  
    std::cout << "scores[\"bob\"] = " << scores["bob"] << "\n";  
    return 0;  
}  
  
/*  
  
###  
lldb .\build\debug\app.exe  
breakpoint set --file main.cpp --line 33  
command script import lldb_task_formatter.py  
type summary add Task -F lldb_task_formatter.task_summary  
run  
  
###  
C:\msys64\clang64\bin\gdb.exe -q -x gdb_task_formatter.py .\build\debug\app.exe  
  
C:\msys64\clang64\bin\gdb.exe .\build\debug\app.exe  
source gdb_task_formatter.py  
break main.cpp:33  
run  
  
  
*/
```

### gdb_task_formatter.py
```python
"""  
GDB pretty-printer для struct Task { std::string name; int priority; }  
  
Загрузка:  
  (gdb) source gdb_printers.pyили в .gdbinit:  
  source gdb_printers.py"""  
import gdb  
  
class TaskPrinter:  
    def __init__(self, val):  
        self.val = val  
  
    def to_string(self):  
        name = str(self.val['name'])  
        priority = int(self.val['priority'])  
        priority_label = {1: 'HIGH', 2: 'MEDIUM', 3: 'LOW'}.get(priority, 'UNKNOWN')  
        return f'Task({name}, priority={priority_label})'  
  
def task_lookup(val):  
    if str(val.type.strip_typedefs()) == 'Task':  
        return TaskPrinter(val)  
    return None  
  
gdb.pretty_printers.append(task_lookup)
```

### lldb_task_formatter.py
```python
"""  
LLDB summary provider для struct Task { std::string name; int priority; }  
  
Загрузка:  
  (lldb) command script import lldb_formatters.py  (lldb) type summary add Task -F lldb_formatters.task_summary  
Можно также автозагрузить через ~/.lldbinit:  
  command script import /path/to/lldb_formatters.py  type summary add Task -F lldb_formatters.task_summary"""  
  
def task_summary(valobj, internal_dict):  
    name = valobj.GetChildMemberWithName('name').GetSummary()  
    priority = valobj.GetChildMemberWithName('priority').GetValueAsSigned()  
    priority_label = {1: 'HIGH', 2: 'MEDIUM', 3: 'LOW'}.get(priority, 'UNKNOWN')  
    return f'Task({name}, priority={priority_label})'
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

## Итоговая сводка по разделу

| |GDB|LLDB|
|---|---|---|
|Механизм|Python pretty-printers, регистрируются через `gdb.pretty_printers`|Python summary/synthetic providers через `type summary add`|
|STL из коробки|✅ через auto-load `.debug_gdb_scripts`|✅ встроено в сам LLDB (data formatters)|
|`unique_ptr` разыменование|✅ надёжно|⚠️ известная проблема на Linux/libstdc++|
|Кастомный тип|Пишешь `.py`, `source` или auto-load|Пишешь `.py`, `command script import`|
