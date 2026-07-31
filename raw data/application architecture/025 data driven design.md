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
        },        
        {            
	        "name": "release",  
            "inherits": "base",  
            "cacheVariables": {  
                "CMAKE_BUILD_TYPE": "Release"  
            }  
        }    
    ],    
    "buildPresets": [  
        {            
	        "name": "debug",  
            "configurePreset": "debug"  
        },  
        {            
	        "name": "release",  
            "configurePreset": "release"  
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
// Data-Driven design: сравниваем хардкод (новый тип врага = новая ветка  
// кода + пересборка) с данными из внешнего файла (новый тип врага =  
// правка enemies.csv, УЖЕ СКОМПИЛИРОВАННЫЙ бинарник подхватывает его  
// без единой пересборки - это и проверяется в самом конце).  
  
#include <sstream>  
#include <fstream>  
#include <iostream>  
#include <format>  
#include <stdexcept>  
#include <string>  
#include <vector>  
  
namespace {  
    struct Enemy {  
        std::string name;  
        int health;  
        int damage;  
        float speed;  
    };}  
  
// ============================================================  
// Вариант 1 (хардкод): каждый новый тип врага - новая ветка в коде.  
// ============================================================  
namespace hardcoded {  
    namespace {  
        enum class EnemyType { Goblin, Orc, Dragon };  
    }
  
    static Enemy createEnemy(const EnemyType type) {  
        switch (type) {  
            case EnemyType::Goblin: return {.name = "Goblin", .health = 20, .damage = 3, .speed = 1.5f};  
            case EnemyType::Orc: return {.name = "Orc", .health = 50, .damage = 8, .speed = 1.0f};  
            case EnemyType::Dragon: return {.name = "Dragon", .health = 500, .damage = 40, .speed = 0.8f};  
        }        
        
        throw std::runtime_error("unknown enemy type");  
        // Чтобы добавить Trol - нужно: (1) добавить значение в enum,  
        // (2) добавить case в switch, (3) пересобрать программу. Баланс-дизайнер        // без доступа к компилятору и репозиторию этого сделать не может.    }  
  
    static void demo() {  
        std::cout << "-- hardcoded --\n";  
        for (const auto type: {EnemyType::Goblin, EnemyType::Orc, EnemyType::Dragon}) {  
            const auto [name, health, damage, speed]{createEnemy(type)};  
            std::cout << std::format("  {}: hp={}, dmg={}, speed={}\n", name, health, damage, speed);  
        }    
    }  
}  

// ============================================================  
// Вариант 2 (data-driven): враги - строки в CSV, движок ("спаунер")  
// написан ОДИН РАЗ и не знает заранее, сколько типов врагов будет  
// существовать и какими они окажутся.  
// ============================================================  
namespace data_driven {  
    namespace {  
        class EnemyDatabase {  
        public:  
            // Загружает определения из внешнего файла - это НЕ часть исходного  
            // кода программы, файл можно менять независимо от бинарника.            static EnemyDatabase loadFromCsv(const std::string& path) {  
                EnemyDatabase db;  
                std::ifstream file{path};  
                if (!file) throw std::runtime_error(std::format("cannot open file '{}'", path)); 

                std::string line;  
                // пропускаем заголовок "name,health,damage,speed"  
                std::getline(file, line);  
  
                while(std::getline(file, line)) {  
                    if (line.empty()) continue;  
                    std::istringstream ss(line);  
                    std::string name, health, damage, speed;  
                    std::getline(ss, name, ',');  
                    std::getline(ss, health, ',');  
                    std::getline(ss, damage, ',');  
                    std::getline(ss, speed, ',');  
                    db.definitions_.push_back({  
                        .name = name,  
                        .health = std::stoi(health),  
                        .damage = std::stoi(damage),  
                        .speed = std::stof(speed)});  
                }  
                return db;  
            }  
            Enemy spawn(const std::string& name) const {  
                for (const auto& def: definitions_) if (def.name == name) return def;  
                throw std::runtime_error(std::format("unknown enemy: {}\n", name));  
            }  
            const std::vector<Enemy>& all() const { return definitions_; }  
  
        private:  
            std::vector<Enemy> definitions_;  
        };    
    }  

    static void demo(const std::string& csvPath) {  
        std::cout << std::format("\n-- data_driven (load from '{}')---\n", csvPath);  
        const auto db{EnemyDatabase::loadFromCsv(csvPath)};  
        for (const auto& [name, health, damage, speed]: db.all()) {  
            std::cout << std::format("  {}: hp={}, dmg={}, speed={}\n", name, health, damage, speed);  
        }    
    }  
}  
  
int main(const int argc, char** argv) {  
    hardcoded::demo();  
  
    const std::string CSV_PATH{  
        (argc > 1)  
        ? argv[1]  
        : "C:\\projects\\knowledge_base\\raw data\\application architecture\\code\\025 data driven design\\enemies.csv"  
    };  
    data_driven::demo(CSV_PATH);  
  
    return 0;  
}
```

`hardcoded::createEnemy()` — новый враг требует новое значение `enum`, новый `case` в `switch`, перекомпиляцию. `data_driven::EnemyDatabase` — движок написан один раз и вообще не знает заранее, сколько будет типов врагов и какими они окажутся;

**Прямая связь с ECS**

В примере с `ComponentStorage`/`World` мы вручную писали `world.positions.add(player, {0, 0})` в коде для каждой сущности — это уже наполовину data-driven (компоненты — чистые данные), но сам факт "какие компоненты есть у какой сущности" всё ещё был захардкожен в `main()`. Логичное продолжение — загружать эти же спецификации сущностей (уровень, спавн-лист, стартовая расстановка) из файла точно так же, как здесь загружается `enemies.csv`: движок остаётся неизменным, а весь контент — данные снаружи. Игровые движки в индустрии именно так и работают: level editor сохраняет сцену в JSON/бинарный формат, runtime читает и инстанцирует entity/component по описанию, ни строчки C++ под конкретный уровень не пишется.

**В enterprise — та же идея, другое имя**

Правила скидок, ставки налога по регионам, условия доступности фичи (feature flags), workflow-состояния заявки — классические кандидаты на data-driven подход: вместо `if (region == "EU" && orderTotal > 100) discount = 0.1; else if (...)`, разрастающегося на сотни строк при каждом новом рынке, — таблица правил (`condition`, `action`) и один универсальный интерпретатор, который её обходит. Правки в такую таблицу часто можно вносить бизнес-пользователю через админку, вообще не трогая деплой.

**Где эта идея ломается, если её тянуть слишком далеко**

Обратная сторона — если "просто данные" начинают включать в себя условную логику, ветвления, вложенные выражения ("если X и (Y или Z), то умножить на 1.2, иначе прибавить фиксированную скидку") — по факту строится язык программирования внутри JSON/CSV, только без типов, без отладчика, без тестов, интерпретируемый самописным движком, которым никто толком не занимался как языком (в индустрии это в шутку называют десятым правилом Гринспена: "любая достаточно сложная программа на C или Fortran содержит наполовину написанную, неформально специфицированную, полную багов, медленную реализацию половины Common Lisp"). Если правила регулярно требуют реальной логики, а не просто параметров — иногда честнее оставить это кодом (пусть даже потребует пересборки) или взять готовый embeddable-язык (Lua, встроенный DSL), чем изобретать всё более выразительный формат конфига вручную. Data-driven design хорош для **параметров и структуры** (числа, списки, какие компоненты у чего есть), но не обязан заменять собой любую логику подряд.
