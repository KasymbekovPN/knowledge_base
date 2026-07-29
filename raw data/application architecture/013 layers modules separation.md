---
tags:
  - programming-language
  - architecture
---
[[raw data/application architecture/_|<=]]

### vcpkg.json
```json
{  
    "name": "project",  
    "version": "0.1.0",  
    "dependencies": []  
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
    ],    
    "testPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug",  
            "output": {  
                "outputOnFailure": true  
            }  
        }    
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(modular_demo CXX)  
  
add_subdirectory(domain)  
add_subdirectory(infra_stripe)  
add_subdirectory(app)
```

### app/CMakeLists.txt
```cmake
add_executable(app main.cpp)  
  
# app - обычный потребитель. Явно линкует только infra_stripe (то, что ему  
# реально нужно создать - конкретный gateway); domain приезжает  
# транзитивно, потому что infra_stripe объявил его как PUBLIC.  
target_link_libraries(app PRIVATE infra_stripe)  
target_compile_features(app PUBLIC cxx_std_23)
```

### app/main.cpp
```cpp
#include "domain/order_processor.hpp"  
#include "infra_stripe/stripe_gateway.hpp"  
  
int main() {  
    StripeGateway gateway;  
    OrderProcessor processor{gateway};  
    processor.placeOrder("laptop", 999.99);  
  
    return 0;  
}
```

### infra_stripe/CMakeLists.txt
```cmake
add_library(infra_stripe STATIC  
        src/stripe_gateway.cpp  
)  
  
target_include_directories(infra_stripe  
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  
)  
  
## PUBLIC, а не PRIVATE: публичный заголовок infra_stripe (stripe_gateway.hpp)  
## сам инклудит domain/order_processor.hpp. Значит, любой, кто линкует  
## infra_stripe и инклудит его заголовок, транзитивно нуждается и в domain -  
## CMake должен передать include-пути domain дальше по цепочке. Если бы  
## domain использовался только внутри .cpp (не протекал в публичный  
## заголовок), здесь было бы PRIVATE - и потребители infra_stripe не видели  
## бы domain вообще, даже транзитивно.  
target_link_libraries(infra_stripe PUBLIC domain)  
target_compile_features(infra_stripe PUBLIC cxx_std_23)
```

### infra_stripe/include/infra_stripe/stripe_gateway.hpp
```cpp
// Публичный API infra_stripe. Он ВКЛЮЧАЕТ заголовок domain, потому что  
// StripeGateway реализует IPaymentGateway, объявленный в domain. Именно  
// поэтому в CMake зависимость domain у infra_stripe будет PUBLIC, а не  
// PRIVATE - см. комментарий в infra_stripe/CMakeLists.txt.  
  
#pragma once  
  
#include "domain/order_processor.hpp"  
  
class StripeGateway: public IPaymentGateway {  
public:  
    bool charge(const double amount) override;  
};
```

### infra_stripe/src/stripe_gateway.cpp
```cpp
#include "infra_stripe/stripe_gateway.hpp"  
  
#include <iostream>  
#include <format>  
  
bool StripeGateway::charge(const double amount) {  
    std::cout << std::format("[Stripe SDK] StripeGateway::charge ${}\n", amount);  
    return true;  
}
```

### domain/CMakeLists.txt
```cmake
add_library(domain STATIC  
        src/order_processor.cpp  
        src/current_format.cpp)  
  
target_include_directories(domain  
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src  
)  
target_compile_features(domain PUBLIC cxx_std_23)
```

### domain/include/domain/order_processor.hpp
```cpp
// ПУБЛИЧНЫЙ API модуля domain. Это единственное, что видят потребители  
// библиотеки domain - всё остальное (domain/src/*) им недоступно физически,  
// не только "по договорённости".  
  
#pragma once  
  
#include <string>  
  
class IPaymentGateway {  
public:  
    virtual ~IPaymentGateway() = default;  
    virtual bool charge(const double amount) = 0;  
};  
  
class OrderProcessor {  
public:  
    explicit OrderProcessor(IPaymentGateway& gateway);  
    bool placeOrder(const std::string& item, const double amount);  

private:  
    IPaymentGateway& gateway_;  
};
```

### domain/src/current_format.hpp
```cpp
// ПРИВАТНЫЙ заголовок domain. Используется только внутри самой библиотеки  
// domain (её .cpp файлами). CMake не добавляет эту директорию в include  
// path потребителей библиотеки - значит infra_stripe и app физически  
// не смогут его заинклудить, даже если попытаются.  
  
#pragma once  
#include <string>  
  
std::string formatCurrency(const double amount);
```

### domain/src/current_format.cpp
```cpp
#include "current_format.hpp"  
  
#include <iomanip>  
#include <sstream>  
  
std::string formatCurrency(const double amount) {  
    std::ostringstream oss;  
    oss << "$" << std::fixed << std::setprecision(2) << amount;  
  
    return oss.str();  
}
```

### domain/src/order_processor.cpp
```cpp
#include "domain/order_processor.hpp"  
  
#include "current_format.hpp"  
  
#include <iostream>  
#include <format>  
  
OrderProcessor::OrderProcessor(IPaymentGateway& gateway):  
    gateway_(gateway) {}  
  
bool OrderProcessor::placeOrder(const std::string& item, const double amount) {  
    std::cout << std::format("[domain] order {} for {}\n", item, formatCurrency(amount));  
    bool ok{gateway_.charge(amount)};  
    std::cout << std::format("{}\n", ok ? "confirmed" : "cancelled");  
  
    return ok;  
}
```

Приватный заголовок `domain` физически не виден снаружи — компилятор не может его найти, только с флагом `-I domain/include` (публичный путь), без `-I domain/src`.**Структура проекта — стандартный паттерн `include/` + `src/`**

`domain/include/domain/order_processor.hpp` — публичный API. `domain/src/currency_format.hpp` — деталь реализации. Разница не в имени папки, а в том, что говорит об этом `CMakeLists.txt`:

```
target_include_directories(domain
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
```

`PUBLIC` означает "этот путь добавляется и в саму библиотеку `domain`, и в компиляцию любого таргета, который её линкует" — CMake транзитивно прокидывает `-I domain/include` всем потребителям. `PRIVATE` означает "путь виден только при сборке самой `domain`, наружу не просачивается". Тест выше это доказал буквально: `#include "currency_format.hpp"` вне `domain` не компилируется с ошибкой "No such file or directory" — не потому что "так не принято", а потому что компилятор физически не получил этот `-I` флаг. Граница модуля здесь — не соглашение между разработчиками, а факт, который проверяет сборочная система на каждой компиляции.

**`PUBLIC`/`PRIVATE`/`INTERFACE` в `target_link_libraries` — это про транзитивность, а не про видимость символов**

`infra_stripe` линкует `domain` как `PUBLIC`:

```
target_link_libraries(infra_stripe PUBLIC domain)
```

Причина — не абстрактная "хорошая практика", а конкретный факт: публичный заголовок `infra_stripe/stripe_gateway.hpp` сам инклудит `domain/order_processor.hpp` (потому что `StripeGateway` наследуется от `IPaymentGateway`). Значит, любой код, который инклудит `stripe_gateway.hpp`, транзитивно нуждается в путях `domain` тоже — и `PUBLIC` говорит CMake: "прокинь эту зависимость дальше по цепочке линковки". Если бы `domain` использовался только внутри `.cpp`-файлов `infra_stripe` и не протекал в публичные заголовки — было бы `PRIVATE`, и потребители `infra_stripe` вообще не узнали бы о существовании `domain`, даже транзитивно. Практическое правило простое: смотри, что реально инклудится в публичном заголовке — это и есть `PUBLIC`-зависимость; всё, что используется только в `.cpp`, — `PRIVATE`.

`app` линкует `infra_stripe` как `PRIVATE` — потому что `app` это конечный исполняемый файл, дальше по цепочке линковать некому, разница `PUBLIC`/`PRIVATE` для executable-таргета не имеет практического значения, но привычка ставить `PRIVATE` для исполняемых файлов и осознанно выбирать между `PUBLIC`/`PRIVATE` для библиотек — то, что реально экономит время при росте графа зависимостей до 20-30 модулей.

**Направление зависимостей между слоями — прямое продолжение DIP**

Правило, которое стоит закрепить компоновкой `CMakeLists.txt` верхнего уровня — `domain` не знает вообще ни о ком (`add_subdirectory(domain)` идёт первым и ничего не линкует), `infra_stripe` знает только про `domain`, `app` — про обоих. Это тот же принцип, что обсуждали на DIP: стрелки зависимостей должны указывать в одну сторону — от деталей к абстракциям, от периферии к ядру (иногда это называют "правилом зависимостей" в Clean/Onion Architecture). Если в какой-то момент `domain/CMakeLists.txt` внезапно захочет `target_link_libraries(domain PRIVATE infra_stripe)` — это явный сигнал архитектурной проблемы: ядро начало зависеть от детали, и весь смысл разделения на модули теряется, даже если формально каждый модуль всё ещё лежит в отдельной папке.

**Static vs shared — что меняется дальше**

Все три таргета здесь — `STATIC` библиотеки: код `domain`/`infra_stripe` физически копируется в финальный `app` при линковке, никакой независимой загрузки в рантайме нет. Если заменить `STATIC` на `SHARED`, границы модулей остаются те же, но появляется новый слой сложности — символы нужно явно экспортировать (`__declspec(dllexport)` на Windows или контроль видимости через `-fvisibility=hidden` + `__attribute__((visibility("default")))` на Linux/macOS, часто через сгенерированный CMake-модулем `GenerateExportHeader` заголовок), и модуль можно будет подгружать в рантайме через `dlopen`/`LoadLibrary` — это прямая дорога к следующей теме плана: plugin-архитектуре. Static-библиотеки как в этом примере — правильная отправная точка для изучения самого разделения на модули, а вопрос "грузить ли что-то динамически в рантайме" — отдельное архитектурное решение поверх уже выстроенных границ.
