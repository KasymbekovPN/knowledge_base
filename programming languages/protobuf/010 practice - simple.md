---
tags:
  - protobuf
---
[[programming languages/protobuf/_|<=]]

Modern CMake (4.4) поддерживает `PROTOC_OPTIONS` через новую функцию `protobuf_generate()`. 


### vcpkg
```json
{  
    "name": "protobuf-file-demo",  
    "version": "1.0.0",  
    "dependencies": [  
        "protobuf"  
    ]  
}
```

### CmakePresets.json
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
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 4.4.2)  
project(protobuf_file_demo CXX)  
  
find_package(Protobuf REQUIRED)  
  
# vcpkg's abseil export bakes in a raw MSVC linker flag ("-ignore:4221") into  
# INTERFACE_LINK_LIBRARIES unconditionally. cl.exe forwards unrecognized  
# "-xxx:yyy" flags to link.exe automatically, but plain clang++ does not, so  
# it fails with "unknown argument". Strip it from any imported target that has it.  
get_property(_imported_targets DIRECTORY PROPERTY IMPORTED_TARGETS)  
foreach(_t ${_imported_targets})  
    get_target_property(_libs ${_t} INTERFACE_LINK_LIBRARIES)  
    if(_libs)  
        list(FILTER _libs EXCLUDE REGEX "ignore:4221")  
        set_target_properties(${_t} PROPERTIES INTERFACE_LINK_LIBRARIES "${_libs}")  
    endif()  
endforeach()  
  
set(PROTO_FILES proto/common/address.proto proto/users/user.proto)  
add_library(proto_gen OBJECT ${PROTO_FILES})  
target_link_libraries(proto_gen PUBLIC protobuf::libprotobuf)  
target_compile_features(proto_gen PUBLIC cxx_std_23)  
  
protobuf_generate(  
        TARGET proto_gen  
        LANGUAGE cpp  
        IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/proto  
        PROTOC_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}  
        # флаг нужен только для protobuf < 3.15 (proto3 optional был experimental);  
        # для свежего protobuf из vcpkg этот флаг не требуется и его можно убрать        # PROTOC_OPTIONS --experimental_allow_proto3_optional)  
target_include_directories(proto_gen PUBLIC ${CMAKE_CURRENT_BINARY_DIR})  
  
add_executable(file_demo main.cpp)  
target_link_libraries(file_demo PRIVATE proto_gen)  
target_include_directories(file_demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

### main.cpp
```cpp
#include <fstream>  
#include <iostream>  
#include <format>  
  
#include "users/user.pb.h"  
  
int main() {  
    const std::string path = "user.bin";  
  
    // --- 1. Создаём сообщение ---  
    myapp::User original;  
    original.set_id(101);  
    original.set_name("Bob");  
    original.set_email("bob@example.com");  
    original.add_addresses()->set_city("Paris");  
    original.set_phone("+33123456789");  
  
    std::cout << "Message created:\n" << original.Utf8DebugString() << "\n";  
  
    // --- 2. Сериализуем в файл ---  
    {  
        std::ofstream out(path, std::ios::binary);  
        if (!original.SerializeToOstream(&out)) {  
            std::cerr << "serialization error in file\n";  
            return 1;  
        }    
    }    
    std::cout << "write to file: " << path << "\n";  
  
    // --- 3. Читаем обратно из файла ---  
    myapp::User loaded;  
    {        
	    std::ifstream in(path, std::ios::binary);  
        if (!loaded.ParseFromIstream(&in)) {  
            std::cerr << "file reading error\n";  
            return 1;  
        }    
    }  
    std::cout << "\nRead form file:\n" << loaded.Utf8DebugString() << "\n";  
  
    // --- 4. Проверка round-trip ---  
    bool matches = (original.SerializeAsString() == loaded.SerializeAsString());  
    std::cout << "Round-trip matches: " << (matches ? "Yes" : "NO") << "\n";  
  
    return matches ? 0 : 1;  
}
```
