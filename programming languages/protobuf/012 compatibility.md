---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

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
  
set(PROTO_FILES schema_v1.proto schema_v2_bad.proto schema_v2_good.proto)  
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

### schema_v1.proto
```protobuf
syntax = "proto3";  
package myapp.v1;  
  
// Исходная схема, уже "в проде" — старые клиенты сериализуют именно так.  
message Event {  
  int32 id = 1;  
  string type = 2;  
  int32 retry_count = 3;  
}
```

### schema_v2_bad.proto
```protobuf
syntax = "proto3";  
package myapp.v2bad;  
  
// ОПАСНАЯ эволюция: кто-то удалил retry_count и завёл новое поле priority,  
// по невнимательности использовав тот же номер 3. Оба поля — int32,  
// то есть одинаковый wire_type. protoc это НЕ поймает, т.к. это два разных  
// файла/версии схемы, компилятор не знает об истории поля 3.  
message Event {  
  int32 id = 1;  
  string type = 2;  
  int32 priority = 3;  
}
```

### schema_v2_good.proto
```protobuf
syntax = "proto3";  
package myapp.v2good;  
  
// Безопасная эволюция: поле retry_count убрали из API, но номер 3  
// зарезервирован, чтобы никто случайно не переиспользовал его.  
// Новое поле email получает НОВЫЙ номер 4, а не 3.  
message Event {  
  reserved 3;  
  reserved "retry_count";  
  
  int32 id = 1;  
  string type = 2;  
  string email = 4;  
}
```

### schema_v2_violation.proto
```protobuf
syntax = "proto3";  
package myapp.v2violation;  
  
// Этот файл специально ломает reserved-запрет — protoc должен отказаться  
// его компилировать. Демонстрирует, что компилятор защищает от повторного  
// использования номера/имени ВНУТРИ одного файла (но не между версиями  
// разных файлов во времени — см. schema_v2_bad.proto).  
message Event {  
  reserved 3;  
  reserved "retry_count";  
  
  int32 id = 1;  
  string type = 2;  
  int32 ops = 3; // нарушение reserved 3  
}
```

### main.cpp
```cpp
#include <iostream>  
#include "schema_v1.pb.h"  
#include "schema_v2_good.pb.h"  
#include "schema_v2_bad.pb.h"  
  
int main() {  
    // ===== Сценарий 1: старый клиент (v1) шлёт данные =====  
    myapp::v1::Event v1;  
    v1.set_id(100);  
    v1.set_type("click");  
    v1.set_retry_count(5);  
  
    std::string bytes;  
    v1.SerializeToString(&bytes);  
    std::cout << "--- v1 sent ---\n" << v1.Utf8DebugString() << "\n";  
  
    // --- Безопасная эволюция: новый сервер (v2good) читает старые данные ---  
    myapp::v2good::Event good;  
    good.ParseFromString(bytes);  
    std::cout << "--- v2good read (safe schema) ---\n"  
             << good.Utf8DebugString();  
    std::cout << "email (new field is absent) = \""  
             << good.email() << "\"\n";  
    const auto& unknown =  
      good.GetReflection()->GetUnknownFields(good);  
    std::cout << "unknown fields count = " << unknown.field_count()  
             << "  <- bites retry_count saved as unknown, did not lose\n\n";  
  
    // --- ОПАСНАЯ эволюция: сервер v2bad переиспользовал номер 3 ---  
    myapp::v2bad::Event bad;  
    bad.ParseFromString(bytes);  
    std::cout << "--- v2bad read (unsafe schema, number 3 reused) ---\n"  
             << bad.Utf8DebugString();  
    std::cout << "priority = " << bad.priority()  
        << "  <- This is actually the retry_count=5 from the old client!\n"  
        << "   There is no error: the wire_type for int32 retry_count and int32\n"  
        << "   priority is the same, so the parser silently assigned bytes\n"  
        << "   intended for the other field to the new one.\n\n";  
  
    // ===== Сценарий 2: новый клиент (v2good) шлёт данные со старым сервером =====  
    myapp::v2good::Event g2;  
    g2.set_id(200);  
    g2.set_type("purchase");  
    g2.set_email("a@b.com");  
  
    std::string bytes2;  
    g2.SerializeToString(&bytes2);  
    std::cout << "--- v2good sent (with new field email) ---\n"  
             << g2.Utf8DebugString() << "\n";  
  
    myapp::v1::Event old;  
    bool ok = old.ParseFromString(bytes2);  
    std::cout << "--- v1 (old parser) read ---\n"  
             << "ParseFromString ok=" << ok << "\n"  
             << old.Utf8DebugString()  
             << "retry_count (field email is absent in v1) = "  
             << old.retry_count() << " (default, did not fail)\n"  
             << "  The old client doesn't crash, but it doesn't see anything email — forward compatibility.\n";  
  
    return 0;  
}
```

**Почему нельзя переиспользовать номер поля**

`v2bad` завёл `int32 priority = 3`, не зная, что номер 3 раньше значил `retry_count`. Старый клиент прислал `retry_count=5`. В выводе видно: `v2bad` прочитал это как `priority: 5` — **без единой ошибки**. Парсер смотрит только на tag (номер + wire_type), а не на имя поля; раз `int32` и там, и там — wire_type совпал, байты легли в поле как ни в чём не бывало, только со совершенно другим смыслом. Это тихая логическая порча данных, которую не поймает ни компилятор, ни рантайм — только баг в продакшене.

**`reserved` — защита на уровне компилятора**

`schema_v2_violation.proto` пытался объявить `int32 oops = 3` после `reserved 3;` — protoc отказался компилировать:

```
schema_v2_violation.proto: Field "oops" uses reserved number 3.
```

Это работает только в пределах одного файла/момента времени. `schema_v2_bad.proto` — это отдельный, "не знающий" о `reserved` файл, поэтому такая же ошибка там не сработала бы — отсюда правило: `reserved` нужно ставить сразу, как только поле выводится из схемы, а не полагаться, что кто-то вспомнит про старый номер вручную.

**Безопасное добавление поля**

`v2good` получил `email` под **новым** номером 4, а не переиспользовал 3. В выводе: `v2good` прочитал `id`/`type` корректно, `email` осталось пустым (нормально — отправитель его не знал), а байты `retry_count` не потерялись — видно как `3: 5` в `DebugString()` и `unknown fields count = 1`. Protobuf хранит непонятные полю байты как unknown fields: если это сообщение потом пере-сериализовать (например, в прокси/шине сообщений), они выйдут обратно нетронутыми — данные физически не теряются, просто текущий код их не видит.

**Безопасное удаление поля**

Правильный способ убрать `retry_count` — не удалить строку молча, а сразу зарезервировать номер и имя: `reserved 3; reserved "retry_count";`. Так следующий разработчик физически не сможет по ошибке занять этот номер — protoc не даст, как показано выше.

**Безопасное переименование поля**

Переименование имени поля (не номера) ничего не ломает на wire — имя есть только в сгенерированном коде, в бинарных данных его нет. `retry_count` → `retryCount` → `attempt_count` — все варианты дадут идентичные байты, если номер поля (3) не менялся.

**Forward compatibility (старый код переживает новые данные)**

`v2good` отправил сообщение с `email`. Старый парсер `v1` прочитал его без единой ошибки: `ParseFromString ok=1`, `retry_count` (поля email тогда не было) вернул дефолтное `0`, `email` ушёл в unknown fields (`4: "a@b.com"` в дебаг-выводе) — старый клиент не падает, просто не видит новых данных.

**Итоговое правило**

Номер поля — это постоянный контракт по байтам, не переменная в коде. Единственная безопасная операция с номером — выделить новый и никогда не трогать старый повторно; `reserved` превращает это правило из "помни сам" в "компилятор проверит за тебя" — но только внутри одного файла на момент компиляции, дисциплина между версиями всё равно на разработчике.
