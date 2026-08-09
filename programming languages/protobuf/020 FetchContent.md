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
  "dependencies" : []
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
project(protobuf_fetch_content_demo CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Отключаем ненужное для итогового бинарника: тесты и примеры protobuf
# компилировать не нужно — они не влияют на наш проект, но сильно увеличивают
# время сборки.
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# protobuf по умолчанию форсирует статический CRT (/MT), заменяя /MD в флагах
# компилятора (см. protobuf-src/CMakeLists.txt: protobuf_MSVC_STATIC_RUNTIME).
# Наш триплет vcpkg — x64-windows-static-md — использует динамический CRT (/MD),
# поэтому без этой опции получаем LNK2038/LNK2005 из-за несовпадения RuntimeLibrary.
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v21.12          # закреплённая версия — как в vcpkg.json/lock-файле
        GIT_SUBMODULES ""              # тесты и бенчмарки нам не нужны, submodules пропускаем
)

# Скачивает (при первом configure) и подключает protobuf как обычный
# add_subdirectory — без системного protoc, без vcpkg, без пакетного менеджера ОС.
FetchContent_MakeAvailable(protobuf)

# После FetchContent_MakeAvailable(protobuf) таргеты protobuf::libprotobuf
# и protobuf::protoc уже доступны (их создаёт сам add_subdirectory(protobuf)).
# НО! Удобная функция protobuf_generate() живёт в protobuf-config.cmake,
# который генерируется и подключается только через find_package(protobuf CONFIG) —
# а FetchContent тянет protobuf через add_subdirectory, этот файл не подключается.
# Поэтому вызываем protoc вручную через add_custom_command — по сути то же
# самое, что protobuf_generate() делает под капотом.
set(PROTO_FILES address.proto user.proto)
set(PROTO_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(GENERATED_SRCS
        ${PROTO_GEN_DIR}/address.pb.cc ${PROTO_GEN_DIR}/address.pb.h
        ${PROTO_GEN_DIR}/user.pb.cc    ${PROTO_GEN_DIR}/user.pb.h
)

add_custom_command(
        OUTPUT ${GENERATED_SRCS}
        COMMAND $<TARGET_FILE:protobuf::protoc>
        --cpp_out=${PROTO_GEN_DIR}
        -I ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROTO_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${PROTO_FILES} protobuf::protoc
        COMMENT "Generation C++ code from .proto via protobuf::protoc (FetchContent)"
        VERBATIM
)

add_library(proto_gen OBJECT ${GENERATED_SRCS})
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)
target_include_directories(proto_gen PUBLIC ${PROTO_GEN_DIR})

add_executable(fetch_content_demo main.cpp)
target_link_libraries(fetch_content_demo PRIVATE proto_gen)
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

**Зачем `FetchContent` вместо vcpkg/системного пакета**

`FetchContent_Declare` + `FetchContent_MakeAvailable` скачивает исходники нужной библиотеки на этапе `configure` и подключает через `add_subdirectory` — как если бы ты вручную склонировал зависимость в поддиректорию проекта. Не нужен ни vcpkg (со своим менеджером пакетов и toolchain-файлом), ни системный protobuf-compiler — версия жёстко закреплена прямо в `CMakeLists.txt` через `GIT_TAG`, воспроизводимо на любой машине с доступом к git и компилятором.

```cmake
FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG        v21.12
)
FetchContent_MakeAvailable(protobuf)
```

**Подводный камень, который я поймал вживую**

Ожидал, что после `FetchContent_MakeAvailable(protobuf)` сразу заработает `protobuf_generate()`, как при `find_package(Protobuf)`. Получил ошибку `Unknown CMake command "protobuf_generate"`. Причина — эта функция буквально прописана внутри `protobuf-config.cmake.in`, который конфигурируется и подключается только через `find_package(protobuf CONFIG)`; `add_subdirectory`, который использует `FetchContent`, этот файл не трогает вообще. При этом сами таргеты `protobuf::libprotobuf` и `protobuf::protoc` создаются и доступны — их создаёт напрямую `add_subdirectory(protobuf)`.

Решение (стандартный паттерн community для этой связки) — вызывать `protoc` вручную через `add_custom_command`, используя `$<TARGET_FILE:protobuf::protoc>`:

```cmake
add_custom_command(
  OUTPUT ${GENERATED_SRCS}
  COMMAND $<TARGET_FILE:protobuf::protoc>
          --cpp_out=${PROTO_GEN_DIR} -I ${CMAKE_CURRENT_SOURCE_DIR} ${PROTO_FILES}
  DEPENDS ${PROTO_FILES} protobuf::protoc
  VERBATIM
)
```

Это по сути то же самое, что `protobuf_generate()` делает под капотом — просто без обёртки-удобства.

**Время сборки — главный компромисс**

Полная сборка protobuf 21.12 (`libprotobuf` + `libprotoc` + `protoc`) с нуля заняла ~53 секунды на 2 ядрах в песочнице — на слабой машине или при полной пересборке "с чистого листа" (`rm -rf build/`) это будет происходить каждый раз, если не настроено кэширование (`ccache`, общий `FETCHCONTENT_BASE_DIR` между проектами, или `FETCHCONTENT_SOURCE_DIR_PROTOBUF`, указывающий на локально уже собранную копию — этим приёмом я и ускорил тест). vcpkg в этом смысле выгоднее для больших/частых пересборок: собирает зависимость один раз и кэширует бинарные пакеты.

**Когда что выбирать**

`FetchContent` — хорош для небольших проектов, где не хочется тащить отдельный пакетный менеджер, и версия зависимости должна быть жёстко зашита прямо в CMake без дополнительных файлов (`vcpkg.json`). vcpkg — лучше для больших проектов с множеством зависимостей и частыми пересборками, где важно кэширование собранных бинарников между CI-запусками. Оба подхода дают одинаковый результат — таргеты `protobuf::libprotobuf`/`protobuf::protoc`, просто разным путём их туда доставляют.
