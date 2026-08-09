---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

### vcpkg
```json
{  
  "name" : "protobuf-file-demo",  
  "version" : "1.0.0",  
  "dependencies" : ["protobuf"]  
}
```

### CMakePresets.json
```json
{  
    "version": 6,  
    "configurePresets": [  
        {            
	        "name": "base",  
            "hidden": true,  
            "generator": "Visual Studio 18 2026",  
            "architecture": {  
                "value": "x64",  
                "strategy": "set"  
            },  
            "binaryDir": "${sourceDir}/build/${presetName}",  
            "toolchainFile": "$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",  
            "cacheVariables": {  
                "VCPKG_TARGET_TRIPLET": "x64-windows-static-md"  
            }  
        },        
        {            
	        "name": "debug",  
            "inherits": "base"  
        },  
        {            
	        "name": "release",  
            "inherits": "base"  
        }  
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "configuration": "Debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release",  
            "configuration": "Release"  
        }  
    ]}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)
project(protobuf_classic_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Protobuf REQUIRED) ищет protobuf в системе/vcpkg-toolchain'е.
# После успешного поиска доступны:
#   Protobuf_FOUND, Protobuf_VERSION
#   Protobuf_INCLUDE_DIRS, Protobuf_LIBRARIES   (классические переменные)
#   protobuf::libprotobuf, protobuf::protoc      (современные imported-таргеты)
#   Protobuf_PROTOC_EXECUTABLE                   (путь к самому protoc)
find_package(Protobuf REQUIRED)

message(STATUS "Protobuf version: ${Protobuf_VERSION}")
message(STATUS "protoc: ${Protobuf_PROTOC_EXECUTABLE}")

# Классический макрос: генерирует .pb.cc/.pb.h и кладёт СПИСКИ путей
# в переменные PROTO_SRCS / PROTO_HDRS. В отличие от protobuf_generate(TARGET...)
# ничего сам не привязывает к таргету — добавлять в add_executable нужно вручную.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
        address.proto
        user.proto
)

message(STATUS "Generated .cc: ${PROTO_SRCS}")
message(STATUS "Generated .h:  ${PROTO_HDRS}")

add_executable(classic_demo main.cpp ${PROTO_SRCS} ${PROTO_HDRS})

# Нужно вручную прописать include-путь к CMAKE_CURRENT_BINARY_DIR,
# т.к. именно туда protobuf_generate_cpp() кладёт сгенерированные файлы.
target_include_directories(classic_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(classic_demo PRIVATE protobuf::libprotobuf)
```

### main.cpp
```cpp
#include <iostream>

#include "user.pb.h"

int main() {
    myapp::User user;
    user.set_id(1);
    user.set_name("Classic macro works");
    std::cout << user.Utf8DebugString();

    return 0;
}
```


**`find_package(Protobuf REQUIRED)`**

Ищет установленный Protobuf (через vcpkg-toolchain, системные пути или `CMAKE_PREFIX_PATH`). При успехе даёт три вида артефактов, видно в логе конфигурации:

```
-- Found Protobuf: .../libprotobuf.so (found version "3.12.4")
-- Protobuf версия: ${Protobuf_VERSION} = 3.12.4
-- protoc: ${Protobuf_PROTOC_EXECUTABLE} = .../protoc
```

Классические переменные (`Protobuf_INCLUDE_DIRS`, `Protobuf_LIBRARIES`) — для ручной настройки через `target_include_directories`/`target_link_libraries`; современные imported-таргеты (`protobuf::libprotobuf`, `protobuf::protoc`) — предпочтительный способ, сами тащат за собой нужные include-пути и флаги, достаточно `target_link_libraries(x PRIVATE protobuf::libprotobuf)`. `REQUIRED` означает, что configure-этап упадёт с понятной ошибкой, если protobuf не найден, а не молча продолжит с пустыми переменными.

**`protobuf_generate_cpp(SRCS_VAR HDRS_VAR proto-файлы...)`**

Более старый, но всё ещё широко используемый макрос (в отличие от `protobuf_generate(TARGET ...)`, который мы применяли раньше). В логе видно, во что он разворачивается:

```
-- Сгенерированные .cc: .../address.pb.cc;.../user.pb.cc
-- Сгенерированные .h:  .../address.pb.h;.../user.pb.h
```

Кладёт списки путей к сгенерированным файлам в переданные переменные (`PROTO_SRCS`, `PROTO_HDRS`), но **ничего сам не привязывает** к конкретному таргету — их нужно вручную добавить в `add_executable`/`add_library`:

```cmake
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS address.proto user.proto)
add_executable(classic_demo main_classic.cc ${PROTO_SRCS} ${PROTO_HDRS})
target_include_directories(classic_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(classic_demo PRIVATE protobuf::libprotobuf)
```

**Разница с `protobuf_generate(TARGET ...)` из практики дня 2**

Макрос `protobuf_generate_cpp()` проще для маленьких/учебных проектов — меньше строк, понятнее с первого взгляда. Функция `protobuf_generate(TARGET ...)` более гибкая (умеет `PROTOC_OPTIONS`, разные языки через `LANGUAGE`, автоматически добавляет исходники в таргет через `target_sources`) — но именно поэтому в этой песочнице только она смогла принять флаг `--experimental_allow_proto3_optional`; классический макрос такой возможности не даёт вообще (проверил в исходнике модуля), поэтому пришлось временно убрать `optional` из `address.proto` для этого демо — на реальном свежем protobuf (из vcpkg) это ограничение не всплывёт, т.к. `optional` там не требует экспериментального флага.

**Практический вывод**

Для новых проектов лучше `protobuf_generate(TARGET ...)` — гибче и меньше ручной сборки; `protobuf_generate_cpp()` встречается в большом количестве существующих проектов и туториалов, полезно уметь его читать, но для нового кода предпочтительнее current-функция.
