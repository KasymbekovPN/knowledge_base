---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

**TLV — структура одного поля**

```
[tag][value]              — для varint / fixed32 / fixed64
[tag][length][value]      — для length-delimited (string, bytes, message)
```

`tag = (номер_поля << 3) | wire_type` — один varint, в котором закодированы сразу два числа.

**Varint — переменная длина числа**

Число разбивается на группы по 7 бит. Каждый байт хранит 7 бит данных + 1 старший "continuation bit" (1 — есть продолжение, 0 — последний байт). Байты идут little-endian по группам (младшие 7 бит — первыми).

Видно на `a=300`: `08 ac 02`. `08` — tag (поле 1, wire_type 0). Значение `300 = 0b1_0010_1100`. Разбиваем по 7 бит с конца: `010_1100` и `10`. Первый байт: `1010_1100 = 0xac` (continuation bit=1, данные `0101100`). Второй байт: `0000_0010 = 0x02` (continuation bit=0, данные `0000010`). Собираем: `0000010` + `0101100` = `100101100` = 300. Сошлось.

`a=1` даёт всего 1 байт значения (`08 01`) — числа 0-127 всегда влезают в один байт varint.

**Проблема с обычным `int32`/`int64` для отрицательных чисел**

Самое важное в выводе — `a=-1 (int32)` занял **11 байт**: `08 ff ff ff ff ff ff ff ff ff 01`. Причина: в protobuf `int32`/`int64` без zigzag кодируют отрицательное число как **64-битное** дополнение до двух (все биты — единицы для -1), и varint честно тащит все эти единицы, обрубая только по 7 бит за раз — получается 10 байт данных вместо ожидаемых "компактных". Это исторический компромисс protobuf: `int32`/`int64` эффективны только для положительных чисел.

**Zigzag — решение для sint32/sint64**

`b=-1 (sint32)` — всего **2 байта**: `10 01`. Zigzag перед varint-кодированием "разворачивает" знаковые числа в беззнаковые так, чтобы малые по модулю числа (и положительные, и отрицательные) оставались малыми:

```
zigzag(n) = (n << 1) ^ (n >> 31)   // для 32-бит, арифметический сдвиг вправо
```

Формула на практике даёт последовательность: `0→0, -1→1, 1→2, -2→3, 2→4, -3→5...` — именно это видно в выводе: `b=0`→ значение 0 (поле вообще не пишется, дефолт), `b=1`→`02` (varint от 2), `b=-2`→`03` (varint от 3), `b=2`→`04` (varint от 4). Знак "закодирован" в младшем бите, а не в старших битах всего 64-битного числа — поэтому маленькие отрицательные числа остаются компактными.

**`fixed32`/`fixed64` — альтернатива varint**

`e=1 (fixed32)` — **всегда 5 байт** (`2d` tag + 4 байта значения little-endian: `01 00 00 00`), независимо от величины числа. Не использует varint вообще (wire_type 5). Смысл: если числа обычно большие (близкие к максимуму int32/int64), varint даст 5 байт на значение всё равно, а fixed32 — гарантированно 4, то есть выгоднее избегать оверхеда varint-кодирования.

**Практическое правило выбора типа**

- Числа обычно маленькие и неотрицательные (id, счётчики, флаги, размеры) → `int32`/`int64`.
- Числа часто отрицательные, но небольшие по модулю (дельты, координаты со знаком) → `sint32`/`sint64`.
- Числа обычно большие (близкие к пределу типа, хэши, случайные ID) → `fixed32`/`fixed64`, `sfixed32`/`sfixed64`.

### vcpkg.json
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
project(protobuf_file_demo CXX)  
  
find_package(Protobuf REQUIRED)  
  
set(PROTO_FILES numbers.proto)  
add_library(proto_gen OBJECT ${PROTO_FILES})  
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)  
target_compile_features(proto_gen PUBLIC cxx_std_23)  
  
protobuf_generate(  
        TARGET proto_gen  
        LANGUAGE cpp  
        IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}  
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}  
)  
target_include_directories(proto_gen PUBLIC ${CMAKE_CURRENT_BINARY_DIR})  
  
add_executable(file_demo main.cpp)  
target_link_libraries(file_demo PRIVATE proto_gen)  
target_include_directories(file_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

### numbers.proto
```protobuf
syntax = "proto3";  
  
package myapp;  
  
message Numbers{  
  int32 a = 1; // varint, без zigzag  
  sint32 b = 2; // varint + zigzag  
  int64 c = 3; // varint, без zigzag  
  sint64 d = 4; // varint + zigzag  
  fixed32 e = 5; // всегда 4 байта, без varint  
}
```

### main.cpp
```cpp
/*  
protoc -I. --cpp_out=. numbers.proto  
*/  
  
#include <iomanip>  
#include <iostream>  
#include <format>  
  
#include "numbers.pb.h"  
  
namespace {  
    void dump(const std::string& label, const myapp::Numbers& numbers) {  
        std::string bytes;  
        numbers.SerializeToString(&bytes);  
        std::cout << std::format("{} -> {} bytes:\n", label, bytes.size());  
        for (unsigned char c: bytes) {  
            std::cout  
                << std::hex  
                << std::setw(2)  
                << std::setfill('0')  
                << static_cast<int>(c) << " ";  
        }        std::cout << std::dec << '\n';  
    }}  
  
int main() {  
    // --- 1. Малое положительное число: varint даёт 1 байт ---  
    {  
      myapp::Numbers n;  
      n.set_a(1);  
      dump("a=1        (int32)", n);  
    }  
    // --- 2. int32 = -1: БЕЗ zigzag varint кодирует как 64-битное число,  
    //        т.к. отрицательные int32 сначала расширяются до int64 —    
    //        получаем классические 10 байт "все единицы" ---    
    {  
      myapp::Numbers n;  
      n.set_a(-1);  
      dump("a=-1       (int32, without zigzag!)", n);  
    }  
    // --- 3. sint32 = -1: С zigzag компактно кодируется в 1 байт ---  
    {  
      myapp::Numbers n;  
      n.set_b(-1);  
      dump("b=-1       (sint32, with zigzag)", n);  
    }  
    // --- 4. sint32 небольшие значения по возрастанию модуля ---  
    {  
      myapp::Numbers n;  
      n.set_b(0);  
      dump("b=0        (sint32)", n);  
    }    
    {      myapp::Numbers n;  
      n.set_b(1);  
      dump("b=1        (sint32)", n);  
    }    
    {      myapp::Numbers n;  
      n.set_b(-2);  
      dump("b=-2       (sint32)", n);  
    }    
    {      myapp::Numbers n;  
      n.set_b(2);  
      dump("b=2        (sint32)", n);  
    }  
    // --- 5. Число, требующее 2 байта varint (>127) ---  
    {  
      myapp::Numbers n;  
      n.set_a(300);  
      dump("a=300      (int32)", n);  
    }  
    // --- 6. int64 = -1 без zigzag: 10 байт ---  
    {  
      myapp::Numbers n;  
      n.set_c(-1);  
      dump("c=-1       (int64, with zigzag)", n);  
    }  
    // --- 7. sint64 = -1 с zigzag: 1 байт ---  
    {  
      myapp::Numbers n;  
      n.set_d(-1);  
      dump("d=-1       (sint64, with zigzag)", n);  
    }  
    // --- 8. fixed32 (не varint вообще, всегда 4 байта, little-endian) ---  
    {  
      myapp::Numbers n;  
      n.set_e(1);  
      dump("e=1        (fixed32)", n);  
    }  

    return 0;  
}
```

```
a=1        (int32) -> 2 bytes:  
08 01   
a=-1       (int32, without zigzag!) -> 11 bytes:  
08 ff ff ff ff ff ff ff ff ff 01   
b=-1       (sint32, with zigzag) -> 2 bytes:  
10 01   
b=0        (sint32) -> 0 bytes:  
  
b=1        (sint32) -> 2 bytes:  
10 02   
b=-2       (sint32) -> 2 bytes:  
10 03   
b=2        (sint32) -> 2 bytes:  
10 04   
a=300      (int32) -> 3 bytes:  
08 ac 02   
c=-1       (int64, with zigzag) -> 11 bytes:  
18 ff ff ff ff ff ff ff ff ff 01   
d=-1       (sint64, with zigzag) -> 2 bytes:  
20 01   
e=1        (fixed32) -> 5 bytes:  
2d 01 00 00 00
```
