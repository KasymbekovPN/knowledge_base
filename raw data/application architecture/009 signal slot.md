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
    "dependencies": ["boost-signals2"]  
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
    ]
}
```

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.30)  
project(app CXX)  
  
find_package(boost_signals2 CONFIG REQUIRED)  
  
add_executable(app main.cpp)  
target_link_libraries(app PRIVATE Boost::signals2)  
target_compile_features(app PUBLIC cxx_std_23)
```

### main.cpp
```cpp
// Signal/Slot (Qt-стиль) вне Qt - через boost::signals2. Никакого moc,  
// никакого общего интерфейса Observer - слотом может быть что угодно  
// с подходящей сигнатурой: лямбда, свободная функция, member function.  
  
#include <boost/signals2.hpp>  
#include <iostream>  
#include <format>  
#include <string>  
  
// ---------------------------------------------------------------------------  
// Button - "источник событий". У него просто есть сигнал как обычное  
// поле-член - никакого наследования от QObject/Observer не требуется.  
// ---------------------------------------------------------------------------  
class Button {  
public:  
    // Тип сигнала описывает сигнатуру слотов: void(const std::string&).  
    // Это и есть типобезопасность - в отличие от нашего EventBus, где    // тип события стирался через const void*, здесь всё типизировано    // на этапе компиляции самим boost::signals2::signal<Sig>.    
    boost::signals2::signal<void(const std::string&)> clicked;  
  
    explicit Button(const std::string &name): name_{std::move(name)} {}  
    void click() { clicked(name_); }  
private:  
    std::string name_;  
};  
  
// Слот как обычная свободная функция - никакого общего интерфейса не нужно.  
void onButtonFunctionFreeFunction(const std::string& name) {  
    std::cout << std::format("  [free fn] click: {}\n", name);  
}  
  
// ---------------------------------------------------------------------------  
// Logger - компонент, подписывающийся на сигнал чужого объекта.  
// scoped_connection - RAII: как наш ручной Connection из примера с EventBus,  
// только уже встроен в саму библиотеку, отключается сам в деструкторе.  
// ---------------------------------------------------------------------------  
class Logger {  
public:  
    void subscribe(Button& button) {  
        conn_ = button.clicked.connect([this](const std::string& name) {onClick(name); });  
    }
private:  
    void onClick(const std::string& name) {  
        std::cout << std::format("  [Logger] click holt: {}\n", name);  
    }  
    boost::signals2::scoped_connection conn_;  
};  
  
int main() {  
    Button okButton("OK");  
  
    // 1. Slot - lambda  
    auto conn1 = okButton.clicked.connect([](const std::string& name) {  
        std::cout << std::format("  [lambda] clicked: {}\n", name);  
    });  

    // 2. Slot - free function  
    okButton.clicked.connect(&onButtonFunctionFreeFunction);  
  
    // 3. Slot - via a separate component with automatic shutdown based on lifetime  
    {  
        Logger logger;  
        logger.subscribe(okButton);  
  
        std::cout << "--- click #1 (3 subscribers: lambda, free fn, Logger) ---\n";  
        okButton.click();  
    } // logger уничтожается -> scoped_connection сам отключает слот  
  
    std::cout << "\n--- click #2 (Logger already unsubscribed) ---\n";  
    okButton.click();  
  
    // отключение и вручную работает как обычно  
    conn1.disconnect();  
    std::cout << "\n--- click #3 (lambda is also disabled; only the free function remains) ---\n";  
    okButton.click();  
  
    return 0;  
}
```

**Как Signal/Slot соотносится с классическим Observer**

Классический Observer (GoF) требует общего интерфейса: `Subject` хранит список `Observer*`, у которых есть виртуальный `update()`, и notify — это обход списка с вызовом одного и того же метода на все подряд, а какое именно событие произошло, наблюдатель обычно выясняет сам (проверяя состояние subject или разбирая параметры). Это работает, но платит инфраструктурой: нужен базовый класс `Observer`, каждый подписчик обязан от него унаследоваться, а один subject обычно уведомляет об одном общем "что-то изменилось", а не о конкретном типизированном событии.

Signal/slot убирает требование общего интерфейса совсем. `boost::signals2::signal<void(const std::string&)> clicked` — это не "список Observer*", а типобезопасный список произвольных вызываемых объектов с конкретной сигнатурой. Слотом может быть лямбда, свободная функция, `std::bind` на member function — никто не обязан ни от чего наследоваться. По сути это Observer, реализованный через type erasure (`std::function`-подобный механизм) вместо виртуального наследования — тот же принцип, что мы использовали руками в `EventBus`, только `signals2` даёт это из коробки, плюс типобезопасность на этапе компиляции (в `EventBus` тип события стирался через `const void*` и восстанавливался через `type_index` в рантайме; здесь сигнатура `signal<Sig>` фиксирована статически).

**Отличие от event bus по топологии**

Ключевая разница с `EventBus` из прошлого примера — не в механизме, а в том, где живёт точка подписки. У event bus — один центральный реестр на всё приложение, подписка идёт по типу события, и неважно, кто именно его публикует. У signal/slot сигнал — это поле конкретного объекта (`okButton.clicked`), и подписываешься ты не на "абстрактное событие где-то в системе", а на сигнал именно этой кнопки. Это делает связь явной и локальной (видно в коде, на что именно подписались), но менее развязанной, чем глобальная шина — если нужно слушать клики сотни разных кнопок одним обработчиком, с сигналами придётся подписываться на каждую по отдельности, тогда как event bus обработает это одной подпиской на тип события.

**Применимость вне Qt**

В Qt сигналы/слоты работают через `moc` — препроцессор, который генерирует служебный код для `QObject`-наследников, а `QueuedConnection` (который обсуждали в блоке про event loop) заворачивает вызов слота в событие Qt-очереди, если отправитель и получатель в разных потоках. Это удобно, но жёстко привязывает к Qt-инфраструктуре — обычную библиотеку с чистым C++, без Qt-зависимости, так писать нельзя.

`boost::signals2` даёт тот же паттерн без всякой кодогенерации — чистые шаблоны, header-only (что и позволило скомпилировать пример просто добавив `-I` на заголовки, без линковки). Это делает его нормальным выбором для переиспользуемых библиотек и подсистем, которые не хотят тянуть Qt как зависимость только ради событийной модели. Разница с Qt: `signals2` из коробки потокобезопасен на уровне доступа к списку слотов (мьютекс внутри `signal`), но не даёт автоматической `QueuedConnection` — вызов слота происходит синхронно, в потоке, который вызвал `signal(...)`; если нужна доставка через event loop другого потока — это надо оборачивать самостоятельно (например, `post()` внутрь слота, как обсуждали в теме `EventBus` + `io_context`).

Из практически полезного в примере: `scoped_connection` — это готовый ответ на проблему, которую мы решали вручную своим классом `Connection` в `EventBus` (danging handler на уничтоженный объект). Тест это подтверждает — после выхода `Logger` из scope и второго `click()` его обработчик уже не вызывается, а программа не падает, хотя никакого ручного `disconnect()` в коде `Logger` нет.
