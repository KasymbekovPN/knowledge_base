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
cmake_minimum_required(VERSION 3.40)  
project(abs_temp CXX)  
  
add_executable(app main.cpp)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// ISP: узкие интерфейсы вместо одного "толстого". Демонстрация того, как  
// нарушение ISP на практике перерастает в нарушение LSP - implementer  
// толстого интерфейса, не умеющий что-то из него, вынужден врать (throw/  
// assert в методе, который "должен" отработать по контракту типа).  
  
#include <iostream>  
#include <format>  
#include <stdexcept>  
#include <vector>  
  
// ============================================================  
// Вариант 1 (антипаттерн): один "толстый" интерфейс на все МФУ-подобные  
// устройства.  
// ============================================================  
namespace bad_fat_interface {  
  
    class IMultiFunctionPointer {  
    public:  
        virtual ~IMultiFunctionPointer() = default;  
        virtual void print(const std::string&) = 0;  
        virtual void scan(const std::string&) = 0;  
        virtual void fax(const std::string&) = 0;  
        virtual void staple() = 0;  
    };
      
    // Простой принтер физически не умеет ни сканировать, ни факсить, ни  
    // сшивать - но интерфейс ЗАСТАВЛЯЕТ реализовать все четыре метода.    
    class BasicPrinter: public IMultiFunctionPointer {  
    public:  
        void print(const std::string& doc) override {  
            std::cout << std::format("  print: {}\n", doc);  
        }        
        void scan(const std::string&) override {  
            throw std::logic_error("BasicPrinter can not scan");  
        }        
        void fax(const std::string&) override {  
            throw std::logic_error("BasicPrinter can not fax");  
        }        
        void staple() override {  
            throw std::logic_error("BasicPrinter can not staple");  
        }    
    };  
    // Клиентский код честно программирует ПРОТИВ интерфейса IMultiFunctionPrinter -  
    // он имеет полное право вызвать scan(), раз тип это объявляет.    
    static void processDocument(IMultiFunctionPointer& device, const std::string& doc) {  
        device.print(doc);  
        // компилируется без единого предупреждения...  
        device.scan(doc);  
    }  
    static void run() {  
        std::cout << "--- bad_fat_interface ---\n";  
        BasicPrinter printer;  
        try {  
            processDocument(printer, "report.pdf");  
        } catch (const std::exception& e) {  
            std::cout << std::format("  runtime-error: {}\n", e.what());  
        }    
    }
}  
  
// ============================================================  
// Вариант 2: узкие интерфейсы, каждый - одна способность.  
// ============================================================  
namespace good_segregated_interfaces {  
  
    class IPrinter {  
    public:  
        virtual ~IPrinter() = default;  
        virtual void print(const std::string&) = 0;  
    };  

    class IScanner {  
    public:  
        virtual ~IScanner() = default;  
        virtual void scan(const std::string&) = 0;  
    };  

    class IFax {  
    public:  
        virtual ~IFax() = default;  
        virtual void fax(const std::string&) = 0;  
    };  

    // BasicPrinter реализует РОВНО то, что умеет - никаких лишних методов,  
    // никакого вранья контракту. У него физически нет метода scan() -    // попытка его вызвать - ошибка КОМПИЛЯЦИИ, а не рантайма.    
    class BasicPrinter: public IPrinter {  
    public:  
        void print(const std::string& doc) override {  
            std::cout << std::format("  print: {}\n", doc);  
        }    
    };  
    
    // AllInOnePrinter честно комбинирует несколько узких интерфейсов через  
    // множественное наследование - в C++ это дёшево и безопасно, когда базовые    // классы чисто абстрактные (без данных - делить нечего, ромбовидной    // проблемы наследования тут просто неоткуда взяться).    
    class AllInOnePrinter: public IPrinter, public IScanner, public IFax {  
    public:  
        void print(const std::string& doc) override {  
            std::cout << std::format("  print: {}\n", doc);  
        }        
        void scan(const std::string& doc) override {  
            std::cout << std::format("  scan: {}\n", doc);  
        }        
        void fax(const std::string& doc) override {  
            std::cout << std::format("  fax: {}\n", doc);  
        }    
    };  
    
    // Клиентский код теперь честен по отношению к тому, что ему реально нужно -  
    // эта функция физически не может вызвать scan() на объекте, у которого    // его нет, потому что принимает только IPrinter&.    
    static void printOnly(IPrinter& printer, const std::string& doc) {  
        printer.print(doc);  
    }  
    
    // А эта функция явно требует и печать, и скан - видно прямо из сигнатуры,  
    // какие способности нужны, без домыслов о том, "а вдруг он ещё что-то умеет".    
    static void printAndScan(IPrinter& printer, IScanner& scanner, const std::string& doc) {  
        printer.print(doc);  
        scanner.scan(doc);  
    }  
    
    static void run() {  
        std::cout << "\n--- good_segregated_interfaces ---\n";  
        BasicPrinter basic;  
        AllInOnePrinter allInOne;  
  
        // ок - BasicPrinter умеет печатать  
        printOnly(basic, "report.pdf");  
        // ок - AllInOnePrinter тоже IPrinter  
        printOnly(allInOne, "report.pdf");  
  
        // ок - есть оба интерфейса  
        printAndScan(allInOne, allInOne, "report.pdf");  
  
        // <- НЕ СКОМПИЛИРУЕТСЯ:  
        // printAndScan(allInOne, basic, "report.pdf");  
        std::cout << std::format("\n  sizeof(BasicPrinter) = {} (1 vptr)\n", sizeof(BasicPrinter));  
        std::cout << std::format("\n  sizeof(AllInOnePrinter) = {} (3 vptr)\n)", sizeof(AllInOnePrinter));  
    }  
}  
  
int main() {  
    bad_fat_interface::run();  
    good_segregated_interfaces::run();  
  
    return 0;  
}
```

**Суть принципа и как он проявляется в коде**

Клиент не должен зависеть от методов, которые ему не нужны. `IMultiFunctionPrinter` с четырьмя методами вынуждает `BasicPrinter` — устройство, которое физически умеет только печатать, — реализовать `scan()`/`fax()`/`staple()` хоть чем-то. В демо это `throw` — и вот здесь ISP напрямую перетекает в нарушение LSP, о котором говорили раньше: `processDocument(IMultiFunctionPrinter&)` имеет полное право вызвать `scan()`, раз тип это заявляет в своём публичном контракте — компилятор пропускает вызов без единого предупреждения, а ошибка вылезает только в рантайме, причём может — в реальном проекте — вылезти не сразу на разработке, а на проде, на конкретном экземпляре `BasicPrinter`, о существовании которого автор `processDocument` мог даже не знать.

**Что меняет разбиение на узкие интерфейсы**

`IPrinter`/`IScanner`/`IFax` — по одной способности на интерфейс. `BasicPrinter` реализует только `IPrinter` — у него физически нет метода `scan()`, соврать контракту ему просто нечем. Ключевой эффект — не эстетический, а измеримый компилятором: `printAndScan(basic, basic, ...)` не компилируется вообще, ошибка ловится на этапе сборки с точным указанием, чего не хватает (`invalid initialization of reference of type IScanner&`). Это тот же сдвиг "ошибка в рантайме → ошибка на этапе компиляции", который был центральной темой почти всех прошлых практик — pImpl, plugin-версионирование, DIP-разделение доменного и инфраструктурного кода. ISP — ещё один инструмент того же принципа: чем раньше несовместимость обнаруживается, тем дешевле её найти.

**Множественное наследование от чисто абстрактных классов — не страшно**

`AllInOnePrinter : public IPrinter, public IScanner, public IFax` компилируется и работает без единой оговорки про ромбовидное наследование — потому что делить нечего: ни у одного базового класса нет данных, только чисто виртуальные методы. Цена видна в `sizeof`: `BasicPrinter` — 8 байт (один vptr), `AllInOnePrinter` — 24 байта (три vptr, по одному на каждый полиморфный базовый подобъект — компилятору нужно отдельное смещение vtable под каждый интерфейс, чтобы `IScanner*`, полученный из `AllInOnePrinter*`, указывал на корректно настроенную vtable именно для `IScanner`). Прямая связь с прошлым разговором про cost виртуальных вызовов: узкие интерфейсы почти бесплатны с точки зрения производительности вызова (тот же косвенный `call` через vtable), но у объектов, реализующих сразу несколько интерфейсов, накапливается по одному vptr на каждый — обычно несущественно, но стоит держать в голове, если таких объектов миллионы.

**Связь с DIP и модульностью из прошлых блоков**

`OrderProcessor` из примера про DIP зависел от узкого `IPaymentGateway` с одним методом `charge()` — не от гипотетического `IExternalServices` со всеми внешними интеграциями сразу. Это ISP в действии: доменный модуль объявляет ровно тот интерфейс, который ему нужен, а не берёт готовый "толстый" интерфейс инфраструктуры целиком. Так же и с публичными заголовками библиотек — узкий интерфейс означает меньше транзитивных зависимостей в `#include`, меньше поводов для пересборки при изменении несвязанной функциональности где-то в "толстом" интерфейсе.
