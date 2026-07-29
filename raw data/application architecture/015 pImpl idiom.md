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
cmake_minimum_required(VERSION 3.16)  
project(pimpl_demo CXX)  
  
add_library(widget STATIC src/widget.cpp)  
target_include_directories(widget PUBLIC  
        ${CMAKE_CURRENT_SOURCE_DIR}/include  
)  
target_compile_features(widget PUBLIC cxx_std_23)  
  
add_executable(app src/main.cpp)  
target_link_libraries(app PRIVATE widget)
```

### src/main.cpp
```cpp
#include "widget.hpp"  
  
#include <iostream>  
#include <format>  
  
int main() {  
    Widget w;  
    w.setName("gadget");  
    std::cout << std::format("{}\n", w.describe());  
    std::cout << std::format("{}\n", w.describe());  
    std::cout << std::format("sizeof(Widget) = {} bytes\n", sizeof(Widget));  
  
    return 0;  
}
```

### include/widget.hpp
```cpp
#pragma once  
  
#include <memory>  
#include <string>  
  
// Публичный заголовок НЕ содержит ни одного #include под детали реализации -  
// никакого <vector>, никакого стороннего SDK, ничего, что мог бы захотеть  
// поменять автор реализации. Потребитель компилирует этот файл быстро и  
// физически не видит внутренностей Widget - только объявление указателя.  
class Widget {  
public:  
    Widget();  
    // объявлен здесь, но НЕ "= default" - Impl пока неполный тип,  
    // компилятор не может сгенерировать деструктор без его определения    
    ~Widget();  
  
    // pImpl-классы либо некопируемы (тогда ничего доп. писать не нужно -  
    // unique_ptr сам запретит копирование), либо явно movable - здесь move    
    // объявлен явно, чтобы Widget можно было класть в std::vector<Widget>.    
    Widget(Widget&&) noexcept;  
    Widget& operator=(Widget&&) noexcept;  
    Widget(const Widget&) = delete;  
    Widget& operator=(const Widget&) = delete;  
  
    void setName(const std::string& name);  
    std::string describe() const;  
  
private:  
    struct Impl;  
    std::unique_ptr<Impl> impl_;  
};
```

### src/widget.cpp
```cpp
#include "widget.hpp"  
  
#include <format>  
  
// Impl определён ЗДЕСЬ, в .cpp - потребитель заголовка widget.hpp никогда  
// не видит этот код и не пересобирается, если тут что-то меняется.  
struct Widget::Impl {  
    std::string name{"unnamed"};  
    int callCount{0};  
};  
  
Widget::Widget(): impl_{std::make_unique<Impl>()} {}  
// теперь Impl полный тип - деструктор генерируется здесь  
Widget::~Widget() = default;  
Widget::Widget(Widget&&) noexcept = default;  
Widget& Widget::operator=(Widget&&) noexcept = default;  
  
void Widget::setName(const std::string& name) { impl_->name = name; }  
  
std::string Widget::describe() const {  
    ++impl_->callCount;  
    return std::format("{} (calling of describe: {})\n", impl_->name, impl_->callCount);  
}
```

**Как это устроено**

`widget.hpp` объявляет `struct Impl;` — неполный (incomplete) тип, просто имя без определения — и хранит `std::unique_ptr<Impl> impl_` как единственное поле. Компилятору достаточно знать размер указателя (`unique_ptr<T>` — это, по сути, обёртка над одним `T*`), чтобы посчитать `sizeof(Widget)` — определение самого `Impl` ему для этого не требуется. Реальное содержимое `Impl` — приватная деталь, объявленная целиком внутри `widget.cpp`, куда потребитель заголовка вообще не заглядывает.

Тонкость, которую легко пропустить: деструктор `~Widget()` нельзя писать `= default` прямо в заголовке — на этом месте `Impl` ещё неполный тип, а `unique_ptr<Impl>` при уничтожении обязан вызвать `delete` на `Impl*`, для чего нужен полный тип с известным размером и деструктором. Поэтому в `widget.hpp` — только объявление (`~Widget();`), а `= default` — в `widget.cpp`, после определения `struct Widget::Impl`, где тип уже полный. То же самое правило касается move-конструктора/присваивания, если их определяют через `= default`.

**Ускорение пересборки — измерено, не на словах**

Тест показал: после того как `Impl` "распух" на два новых поля и метод (`history`, `createdAt`, `recordCall()`) без единого изменения в `widget.hpp`, объектный файл `main.cpp.o` остался **байт-в-байт тем же** (совпал MD5-хэш до и после `cmake --build`) — компилятор его даже не трогал, пересобрался только `widget.cpp.o`. В реальном проекте, где от заголовка какого-нибудь `Renderer` или `NetworkClient` зависят полсотни файлов по всей кодовой базе, это разница между "поправил приватную деталь — жду пересборки всего проекта" и "поправил приватную деталь — пересобрался один файл". Это прямое продолжение темы модульности из прошлого раза: pImpl — это ещё один инструмент того же принципа "публичный интерфейс должен быть маленьким и стабильным", только на уровне отдельного класса, а не целой библиотеки.

**ABI-стабильность**

`sizeof(Widget)` остался 8 байт (один указатель) в обеих версиях, несмотря на то что `Impl` вырос. Это критично для бинарной совместимости: если `Widget` — часть публичного API `.so`/`.dll`, а приложение, которое её использует, скомпилировано против старого заголовка и не пересобирается при каждом обновлении библиотеки (типичная ситуация для системных библиотек или плагинов, о которых говорили в прошлый раз) — добавление новых полей напрямую в класс изменило бы `sizeof` и layout памяти, и старый бинарник начал бы работать с объектом неправильного размера — классическая причина повреждения памяти при обновлении shared-библиотеки без пересборки всех её потребителей. С pImpl потребитель всегда видит только указатель, а что за ним — может меняться от версии к версии библиотеки, не трогая ABI.

**Цена**

Это не бесплатно: каждый объект `Widget` требует отдельного выделения в куче под `Impl` (`make_unique`) — лишний `new`/`delete` и промах кэша при разыменовании `impl_->` по сравнению с прямыми полями-членами. Для класса, которого создаются миллионы за кадр (например, в игровом движке в hot path) — это может быть измеримо дороже. pImpl обычно оправдан для классов уровня "публичный API библиотеки/подсистемы, создаётся не в горячем цикле" — ровно то, что было в примере (`Widget` как публичный тип библиотеки), а не для внутренних structs, которые создаются тысячами в секунду.
